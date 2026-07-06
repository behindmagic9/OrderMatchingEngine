#include<iostream>
#include<queue>
#include<map>
#include<unordered_map>
#include<algorithm>
#include<vector>
#include<list>
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
    CANCELLED,
    REJECTED
};

struct Order {
    int orderId;
    char side;
    int price;
    int quantity;
    Status status;
};

struct Trade {
    int BuyId;
    int SellId;
    int quant;
    int price;
};

struct OrderRef{
    int price;
    char side;
    std::list<Order>::iterator iterator;
};

std::vector<Trade> trades;
std::queue<Order> qe;

std::map<int, std::list<Order>, std::greater<int>> BUY;
std::map<int, std::list<Order>> SELL;

// cancell order , for that have to track each orders pointer in a seprate record , and will directly remove that pointer from that side of map in o(1)
//  as the second part also contain queue and queue does not have O(1) removal so we will find the iterator to it and then remove that instantly
std::unordered_map<int, OrderRef> OrderPointersStore; // orderid, iterator

void Producer() {
    int id = 1;
    qe.push(Order{ id++,'B',105, 100,Status::NEW});
    qe.push(Order{ id++,'S',103, 100,Status::NEW});
    qe.push(Order{ id++,'B',105,100,Status::NEW});
    qe.push(Order{ id++,'S',103, 40,Status::NEW});
    qe.push(Order{ id++,'B',100,100,Status::NEW});
    qe.push(Order{ id++,'S',101, 100,Status::NEW});
    qe.push(Order{ id++,'B',105,100,Status::NEW});
    qe.push(Order{ id++,'S',100, 100,Status::NEW});
}   

void RecordTrade(Order &incoming, Order &recieving , int quantity , int price){
    Trade t{
        incoming.side == 'B' ? incoming.orderId : recieving.orderId,
        incoming.side == 'S' ? incoming.orderId : recieving.orderId,
        quantity,
        price
    };
    trades.push_back(t);
}

bool Validator(Order& order){
    if(order.orderId <= 0) return false;
    if(order.quantity <= 0) return false;
    if(order.price <=0) return false;
    if (order.side != 'B' && order.side != 'S') return false;
    return true;
}


void AddToOrderBook(const Order &order){
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

template<typename OppositeBook, typename Compare>
void MatchOrder(Order &order, OppositeBook &oppositeBook, Compare comp){
    int originalQuantity = order.quantity;
    auto it = oppositeBook.begin();
    while (order.quantity > 0 && it != oppositeBook.end() && comp(order.price,it->first)) {
            // break when sell price is greater than order buy price cause , terminate condition is (sell > buy)
            // will help in optimization too not loop in map ,and directly come to conclusion
            // and also help in breaking while LOOP

        auto &q = it->second;
        while (order.quantity > 0 && !q.empty()) {
            auto &ord = q.front();
            int quant = std::min(ord.quantity, order.quantity);

            // trade is heppenig so will record trade obejct here
            RecordTrade(order, ord, quant, it->first);

            order.quantity -= quant;    
            ord.quantity -= quant;

            if (ord.quantity == 0) {
                ord.status =  Status::FILLED;
                OrderPointersStore.erase(ord.orderId);
                q.pop_front();
            }else{
                ord.status = Status::PARTIAL_FILLED;
            }
        }
        if (q.empty()) {
            it = oppositeBook.erase(it);
        }
        else {
            break;
        }
        // look into the queue and take the first elem adn match the trade with that
        // Order temp = it->second.front();
        // here ahvet o subtrat this from the quqnity til order complete
        // or just do while(!(order.price < it->first && order.quantity <= 0)){}
        // in mid return if quantity is done , then return the TRade struct
    }

    if (order.quantity == 0){
        order.status = Status::FILLED;
        std::cout << "order fullfilled " << std::endl;
    }else if(order.quantity < originalQuantity){
        // marking this as partial filled
        order.status = Status::PARTIAL_FILLED;
        AddToOrderBook(order);
    }else{
        order.status = Status::OPEN;
        AddToOrderBook(order);
    }
}

void ProcessBUY(Order &order) {
    MatchOrder(order, SELL, 
        [](int buyPrice, int sellPrice){
            return buyPrice >= sellPrice;
        }
    );
}

void ProcessSELL(Order &order) {
    MatchOrder(order, BUY, 
        [](int buyPrice, int sellPrice){
            return buyPrice <= sellPrice;
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
    it.iterator->status = Status::CANCELLED;
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

void ModifyOrder(int orderId , int newprice=-1, int newquantity=-1 ,char newside = '\0'){
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
            if(newquantity <=0){
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
    qe.push(oldOrder);
}
 
void ProcessOrder(Order &order) {
    if (order.side == 'B') {
        // first have to see if the map is null or not
        // if null then inet directly
        if (SELL.empty()) {
            order.status = Status::OPEN;
            std::cout << "directly pushed to buy" << std::endl;
            AddToOrderBook(order);
        }
        else {
            ProcessBUY(order);
        }
    }
    else {
        if (BUY.empty()) {
            order.status = Status::OPEN;
            std::cout << "directly pushed to sell" << std::endl;
            AddToOrderBook(order);
        }
        else {
            ProcessSELL(order);
        }
    }
}

void PrintTrades(){
    for(int i =0;i<trades.size();i++){
        std::cout << "Buyer id : " << trades[i].BuyId << " seller ID : " << trades[i].SellId << " price is: " << trades[i].price << " quantity is: " << trades[i].quant << std::endl;
    }
}

void Consumer() {
    while (!qe.empty()) {
        Order temp = qe.front();
        qe.pop();
        if(!Validator(temp)){
            temp.status = Status::REJECTED;
            std::cout << " validation of order id : " << temp.orderId << " : failed with status : Rejected " << std::endl;
            continue;
        }else{
            ProcessOrder(temp);
        }
        // std::cout << "order id : " << temp.orderId << "side : " << temp.side << " price: " << temp.price << std::endl;
    }
}

void PrintOrderBook(){
    std::cout << "BUY >> ";

    for(const auto &p : BUY){
        int total =0;
        for(const auto &ord : p.second){
            total +=ord.quantity;
        }
        std::cout << "price : " << p.first << " quanitity "  << total << std::endl;
    }  
    
    std::cout << "SELL >> ";
    
    for(const auto &p : SELL){
        int total =0;
        for(const auto &ord : p.second){
            total +=ord.quantity;
        }
        std::cout << "price : " << p.first << " quanitity "  << total << std::endl;
    }
    std::cout << std::endl;
}


int main() {
    Producer();
    Consumer();
    PrintTrades();
    PrintOrderBook();
    CancelOrder(4);
    PrintOrderBook();
    return 0;
}
