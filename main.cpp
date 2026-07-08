#include<iostream>
#include<queue>
#include<map>
#include<unordered_map>
#include<algorithm>
#include<vector>
#include<list>
#include<unordered_set>
// producer
// consumer
// while(true)
// accept it
// we will have a queue and then produer will fill that queue by publihshing a nd consumer will take that queue and consume frmo it
// its all in synchronization or in order

enum class Status{
    NEW,
    OPEN,
    FILLED,
    PARTIAL_FILLED,
    PARTIAL_FILLED_Cancel,
    CANCELLED,
    REJECTED
};

enum class OrderType
{
    Limit,
    Market
};

enum class OrderTimeinFrame
{
    GTC, // Good Till Cancel
    FOK, // Fill or KILL
    IOC  // Immediate or KIll
};

struct Order {
    int orderId;
    char side;
    int64_t price;
    int quantity;
    OrderType otype;
    OrderTimeinFrame otf;
    Status status;
};

struct Trade {
    int BuyId;
    int SellId;
    int quant;
    int64_t price;
};

std::unordered_set<int> orderIds;

struct OrderEvent {
    Status oldStatus;
    Status newStatus;
    int64_t price;
    int originalquantity;
    int execquantity;
    int remquantity;
};

struct OrderRef{
    int64_t price;
    char side;
    std::list<Order>::iterator iterator;
};

struct NewOrder
{
    Order order;
    bool fromReplace = false;
};

struct CancelOrder
{
    int orderId;
};

struct ModifyOrder
{
    int orderId;
    int64_t newprice = -1;
    int newquantity = -1;
    char newside = '\0';
};

enum class CommandType
{
    New,
    Modify,
    Cancel
};

struct Command
{
    CommandType type;

    union
    {
        NewOrder neworder;
        CancelOrder cancelOrder;
        ModifyOrder modifyOrder;
    };

    static Command New(const Order& order , bool replace = false)
    {
        Command c;
        c.type = CommandType::New;
        c.neworder = { order,replace };
        return c;
    }

    static Command Modify(int id, int64_t price = -1, int quanitity = -1, char side = '\0')
    {
        Command c;
        c.type = CommandType::Modify;
        c.modifyOrder = { id, price, quanitity, side };
        return c;
    }

    static Command Cancel(int id)
    {
        Command c;
        c.type = CommandType::Cancel;
        c.cancelOrder = { id };
        return c;
    }

    Command() {}
    ~Command() {}
};

std::unordered_map<int, std::vector<OrderEvent>> orderHistory; // orderid, Orders vector

std::vector<Trade> trades;
std::queue<Command> qe;

std::map<int64_t, std::list<Order>, std::greater<int64_t>> BUY;
std::map<int64_t, std::list<Order>> SELL;

// cancell order , for that have to track each orders pointer in a seprate record , and will directly remove that pointer from that side of map in o(1)
// as the second part also contain queue and queue does not have O(1) removal so we will find the iterator to it and then remove that instantly
std::unordered_map<int, OrderRef> OrderPointersStore; // orderid, iterator

void Producer() {
    int id = 1;
    qe.push(Command::New(Order{ id++, 'B', 105, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    qe.push(Command::New(Order{ id++, 'S', 103, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    qe.push(Command::New(Order{ id++, 'B', 105, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    qe.push(Command::New(Order{ id++, 'S', 103, 40, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    qe.push(Command::New(Order{ id++, 'B', 100, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    qe.push(Command::New(Order{ id++, 'S', 101, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    qe.push(Command::New(Order{ id++, 'B', 105, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    qe.push(Command::New(Order{ id++, 'S', 100, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    qe.push(Command::Modify(5, -1, -1, 'S'));
}

void RecordTrade(Order& incoming, Order& recieving, int quantity, int64_t price){
    Trade t{
        incoming.side == 'B' ? incoming.orderId : recieving.orderId,
        incoming.side == 'S' ? incoming.orderId : recieving.orderId,
        quantity,
        price };
    trades.push_back(t);
}

void RecordOrderEvent(Order& order, Status newStatus, int execquantity=0, int origquantity=-1) {

    if (origquantity == -1) {
        origquantity = order.quantity + execquantity;
    }
    orderHistory[order.orderId].push_back(
        {
            order.status,
            newStatus,
            order.price,
            origquantity,
            execquantity,
            order.quantity,
        }
    );
    order.status = newStatus;
}

bool Validator(const Order& order){
    if(order.orderId <= 0) return false;
    if(order.quantity <= 0) return false;
    if (order.side != 'B' && order.side != 'S') return false;
    if (order.otype == OrderType::Limit && order.price <= 0) return false;
    if (order.otype == OrderType::Market && order.otf == OrderTimeinFrame::GTC) return false;
    return true;
}

void AddToOrderBook(const Order& order){
    if(order.side == 'B'){
        BUY[order.price].push_back(order);
        auto it = std::prev(BUY[order.price].end());
        OrderPointersStore[order.orderId] = {order.price,order.side,it};
    }else{
        SELL[order.price].push_back(order);
        auto it = std::prev(SELL[order.price].end());
        OrderPointersStore[order.orderId] = {order.price,order.side,it};
    }
}

template <typename OppositeBook, typename Compare>
bool CanFullFillOrder(const Order& order, OppositeBook& oppositeBook, Compare comp)
{
    int remaining = order.quantity;
    for (auto it = oppositeBook.begin(); it != oppositeBook.end(); it++)
    {
        if (order.otype == OrderType::Limit && !comp(order.price, it->first))
        {
            break;
        }
        for (auto& ord : it->second)
        {
            remaining -= ord.quantity;
            if (remaining <= 0)
            {
                return true;
            }
        }
    }
    return false;
}

// here the Opposite BOok is deined by type whihc defined on calling the MAtchOrder fucntion
// the compare here work as the Lambda Obejct type whcih deduce the comp with lambda() operator , whihc is what we define in the Calling when ProcessBUy and ProcessSEll of like
/*
class Lambda
{
public: bool Opertor() (int buy price , int sellPrice ){
return ..........
}
// as this condition is differnt in both buy and sell so make it lambda so that can independtly declare and check that and perform correctly
}
*/
template <typename OppositeBook, typename Compare>
void MatchOrder(Order& order, OppositeBook& oppositeBook, Compare comp)
{
    if (order.otf == OrderTimeinFrame::FOK)
    {
        if (!CanFullFillOrder(order, oppositeBook, comp))
        {
            RecordOrderEvent(order, Status::CANCELLED);
            order.quantity = 0;
            return;
        }
    }
    int originalQuantity = order.quantity;
    auto it = oppositeBook.begin();
    while (order.quantity > 0 && it != oppositeBook.end())
    {
        if (order.otype == OrderType::Limit && !comp(order.price, it->first))
        {
            break;
        }
        // rest market order type will skip above condition and jump to this and execute direclty wiht no price thing
        auto& q = it->second;
        while (order.quantity > 0 && !q.empty())
        {
            auto& ord = q.front();
            int quant = std::min(ord.quantity, order.quantity);

            // trade is heppenig so will record trade obejct here
            int64_t tradePrice = ord.price;
            RecordTrade(order, ord, quant, tradePrice);

            order.quantity -= quant;
            ord.quantity -= quant;
            int exec = quant;
            if (ord.quantity == 0) {
                RecordOrderEvent(ord, Status::FILLED,exec);
                OrderPointersStore.erase(ord.orderId);
                q.pop_front();
                break;
            }else{
                RecordOrderEvent(ord, Status::PARTIAL_FILLED,quant);
            }
        }
        if (order.quantity == 0) {
            break;
        }
        if (q.empty()) {
            it = oppositeBook.erase(it);
        }
        else {
            ++it;;
        }
    }

    if (order.quantity == 0){
        RecordOrderEvent(order, Status::FILLED,originalQuantity);
        std::cout << "order fullfilled " << std::endl;
    }else if(order.quantity < originalQuantity)
    {
        if (order.otype == OrderType::Market)
        {
            RecordOrderEvent(order, Status::PARTIAL_FILLED_Cancel, originalQuantity - order.quantity , originalQuantity);
            order.quantity = 0;
        }
        else if (order.otf == OrderTimeinFrame::GTC)
        {
            RecordOrderEvent(order, Status::PARTIAL_FILLED , originalQuantity - order.quantity, originalQuantity);
            // add to otder book
            AddToOrderBook(order);
        }
        else if (order.otf == OrderTimeinFrame::IOC)
        {
            // cancle order now
            std::cout << "filled is : " << originalQuantity - order.quantity << ": remainging are cancelled : " << order.quantity << std::endl;
            RecordOrderEvent(order, Status::PARTIAL_FILLED_Cancel , originalQuantity - order.quantity, originalQuantity);
            order.quantity = 0; // reseting the quantity
        }
    }
    else
    {
        if (order.otype == OrderType::Market)
        {
            RecordOrderEvent(order, Status::CANCELLED,0);
            order.quantity = 0;
        }
        else if (order.otf == OrderTimeinFrame::GTC)
        {
            RecordOrderEvent(order, Status::OPEN);
            // add to otder book
            AddToOrderBook(order);
        }
        else if (order.otf == OrderTimeinFrame::IOC)
        {
            // cancle order now
            RecordOrderEvent(order, Status::CANCELLED,0);
            std::cout << "nothing matched so cancelling it" << std::endl;
            order.quantity = 0; // reseting the quantity
        }
    }
}

void ProcessBUY(Order& order)
{
    MatchOrder(order, SELL,
        [](int64_t buyPrice, int64_t sellPrice)
        {
            return buyPrice >= sellPrice;
        }
    );
}

void ProcessSELL(Order& order)
{
    MatchOrder(order, BUY,
        [](int64_t sellPrice, int64_t buyPrice)
        {
            return buyPrice >= sellPrice;
        }
    );
}

void CancelOrder(int Orderid){
    auto p = OrderPointersStore.find(Orderid);
    if(p == OrderPointersStore.end()){
        std::cout << "return" << std::endl;
        return;
    }
    auto it = p->second;
    RecordOrderEvent(*(it.iterator), Status::CANCELLED,0);
    if(it.side == 'B'){
        std::cout << "from b" << std::endl;
        auto lst = BUY.find(it.price);
        if(lst != BUY.end()){
            lst->second.erase(it.iterator);
            if(lst->second.empty()){
                BUY.erase(lst);
            }
        }
    }else{
        std::cout << "from s" << std::endl;
        auto lst = SELL.find(it.price);
        if(lst != SELL.end()){
            lst->second.erase(it.iterator);
            if(lst->second.empty()){
                SELL.erase(lst);
            }
        }
    }
    OrderPointersStore.erase(p);
}

void ModifyOrder(int orderId, int64_t newprice = -1, int newquantity = -1, char newside = '\0')
{
    auto it = OrderPointersStore.find(orderId);
    if (it == OrderPointersStore.end()){
        return ;// so such order
    }
    Order oldOrder = *(it->second.iterator);
    bool priceChanged = (newprice != -1 && newprice != oldOrder.price);
    bool quantityIncreased = (newquantity != -1 && newquantity > oldOrder.quantity);
    bool sideChanged = (newside !='\0' && newside != oldOrder.side);

    if(!priceChanged  && !quantityIncreased && !sideChanged){
        if(newquantity != -1){
            if(newquantity <= 0){
                std::cout << "invalid quantity" << std::endl;
                return;
            }
            it->second.iterator->quantity = newquantity;
        }
        return;
    }
    if(!(newprice == -1)){
        oldOrder.price = newprice;
    }
    if(!(newquantity == -1)){
        oldOrder.quantity = newquantity;
    }
    if(!(newside == '\0')){
        oldOrder.side = newside;
    }

    if(!Validator(oldOrder)){
     std::cout << " invalid mofications : rejected by validator" << std::endl;
        return;
    }

    //Nee cancel order ad  repalce
    CancelOrder(orderId);
    oldOrder.status = Status::NEW;
    qe.push(Command::New(oldOrder,true));
}

void ProcessOrder(Order& order) {
    if (order.side == 'B')
    {
        ProcessBUY(order);
    }
    else {
        ProcessSELL(order);
    }
}

void PrintTrades(){
    for (const auto& trad : trades)
    {
        std::cout << "Buyer id : " << trad.BuyId << " seller ID : " << trad.SellId << " price is: " << trad.price << " quantity is: " << trad.quant << std::endl;
    }
}

bool DuplicateOrder(int orderId) {
    return orderIds.find(orderId) != orderIds.end();
}

void Consumer() {
    while (!qe.empty()) {
        auto temp = qe.front();
        qe.pop();

        switch (temp.type)
        {
        case CommandType::New:
        {
            Order order = temp.neworder.order;
            if (!Validator(order))
            {
                std::cout << " validation of order id : " << order.orderId << " : failed with status : Rejected " << std::endl;
                RecordOrderEvent(order, Status::REJECTED,0);
                break;
            }
            if (DuplicateOrder(order.orderId) && !temp.neworder.fromReplace) {
                RecordOrderEvent(order, Status::REJECTED, 0);
                break;
            }
            orderIds.insert(temp.neworder.order.orderId);
            ProcessOrder(order);
            break;
        }
        case CommandType::Cancel:
        {
            CancelOrder(temp.cancelOrder.orderId);
            break;
        }
        case CommandType::Modify:
        {
            ModifyOrder(temp.modifyOrder.orderId, temp.modifyOrder.newprice, temp.modifyOrder.newquantity, temp.modifyOrder.newside);
            break;
        }

        // std::cout << "order id : " << temp.orderId << "side : " << temp.side << " price: " << temp.price << std::endl;
        }
    }
}

void PrintOrderBook(){
    std::cout << "BUY >> ";

    for(const auto& p : BUY)
    {
        int total =0;
        for(const auto& ord : p.second)
        {
            total +=ord.quantity;
        }
        std::cout << "price : " << p.first << " quanitity " << total << std::endl;
    }

    std::cout << "SELL >> ";

    for(const auto& p : SELL)
    {
        int total =0;
        for(const auto& ord : p.second)
        {
            total +=ord.quantity;
        }
        std::cout << "price : " << p.first << " quanitity " << total << std::endl;
    }
    std::cout << std::endl;
}

std::string StatusToString(Status status)
{
    switch (status)
    {
    case Status::NEW:
        return "NEW";
    case Status::OPEN:
        return "OPEN";
    case Status::FILLED:
        return "FILLED";
    case Status::PARTIAL_FILLED:
        return "PARTIAL_FILLED";
    case Status::PARTIAL_FILLED_Cancel:
        return "PARTIAL_FILLED_CANCEL";
    case Status::CANCELLED:
        return "CANCELLED";
    case Status::REJECTED:
        return "REJECTED";
    default:
        return "UNKNOWN";
    }
}

void PrintOrderHistory() {
    std::cout << "\tID\t" << "Price\t" << "Original\t" << "Remain Quantity\t" << " Exec Quantity\t\t"  << "Old Status\t" << "New Status\t" << std::endl;
    for (auto it = orderHistory.begin(); it != orderHistory.end(); it++) {
        int orderid = it->first;
        for (const auto& second : it->second) {
            std::cout << "\t" << orderid << "\t" << second.price << "\t" << second.originalquantity << "\t\t" << second.remquantity << "\t\t\t" << second.execquantity << "\t\t" << StatusToString(second.oldStatus) << "\t\t" << StatusToString(second.newStatus) << "\t" << std::endl;
        }
    }
}

int main()
{
    Producer();
    Consumer();
    PrintTrades();
    PrintOrderBook();
    PrintOrderHistory();
    return 0;
}