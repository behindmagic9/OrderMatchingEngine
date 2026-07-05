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

struct Order {
    int orderId;
    char side;
    int price;
    int quantity;
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
    qe.push(Order{ 1,'B',1 * 4, 35 });
    qe.push(Order{ 2,'S',3 * 2, 20 });
    qe.push(Order{ 3,'B',5 * 4, 30 });
    qe.push(Order{ 4,'S',6 * 2, 25 });
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

void ProcessBUY(Order &order) {
    int originalQuantity = order.quantity;
    auto it = SELL.begin();
    while (order.quantity > 0 && it != SELL.end()) {
        if (it->first > order.price) {
            // break when sell price is greater than order buy price cause , terminate condition is (sell > buy)
            // will help in optimization too not loop in map ,and directly come to conclusion
            // and also help in breaking while LOOP
            break;
        }
        auto &q = it->second;
        while (order.quantity > 0 && !q.empty()) {
            auto &ord = q.front();
            int quant = std::min(ord.quantity, order.quantity);

            // trade is heppenig so will record trade obejct here
            RecordTrade(order, ord, quant, it->first);

            order.quantity -= quant;
            ord.quantity -= quant;
            if (ord.quantity == 0) {
                OrderPointersStore.erase(ord.orderId);
                it->second.pop_front();
            }
        }
        if (q.empty()) {
            it = SELL.erase(it);
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
        std::cout << "order fullfilled " << std::endl;
    }else if(order.quantity < originalQuantity){
        // marking this as partial filled
        AddToOrderBook(order);
    }else{
        AddToOrderBook(order);
    }
}

void ProcessSELL(Order &order) {
    int originalQuantity = order.quantity;
    auto it = BUY.begin();
    while (order.quantity > 0 && it != BUY.end()) {
        if (it->first < order.price) {
            // break when sell price is greater than order buy price cause , terminate condition is (sell > buy)
            // will help in optimization too not loop in map ,and directly come to conclusion
            // and also help in breaking while LOOP
            break;
        }
        auto &q = it->second;
        while (order.quantity > 0 && !q.empty()) {
            auto &ord = q.front();
            int quant = std::min(ord.quantity, order.quantity);
            // trade is heppenig so will record trade obejct here
            RecordTrade(order, ord, quant, it->first);
            order.quantity -= quant;
            ord.quantity -= quant;
            if (ord.quantity == 0) {
                OrderPointersStore.erase(ord.orderId);
                it->second.pop_front();
            }
        }
        if (q.empty()) {
            it = BUY.erase(it);
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
        std::cout << "order fullfilled " << std::endl;
    }else if(order.quantity < originalQuantity){
        // marking this as partial filled
        std::cout << "partial filled remain saved to orderbook" << std::endl;
        AddToOrderBook(order);
    }else{
        AddToOrderBook(order);
    }
}

void ProcessOrder(Order &order) {
    if (order.side == 'B') {
        // first have to see if the map is null or not
        // if null then inet directly
        if (SELL.empty()) {
            std::cout << "directly pushed to buy" << std::endl;
            AddToOrderBook(order);
        }
        else {
            ProcessBUY(order);
        }
    }
    else {
        if (BUY.empty()) {
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
            std::cout << " validation of order id : " << temp.orderId << " : failed " << std::endl;
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

void CancelOrder(int Orderid){
    auto p = OrderPointersStore.find(Orderid);
    if(p == OrderPointersStore.end()){
        std::cout << "return" << std::endl;
        return;
    }
    auto &it = p->second;
    if(it.side == 'B'){
        std::cout << "from b" << std::endl;
        auto lst = BUY.find(it.price);
        if(lst != BUY.end()){
            lst->second.erase(it.iterator);
            if(lst->second.empty()){
                BUY.erase(it.price);
            }
        }
    }else{
        std::cout << "from s" << std::endl;
        auto lst = SELL.find(it.price);
        if(lst != SELL.end()){
            lst->second.erase(it.iterator);
            if(lst->second.empty()){
                SELL.erase(it.price);
            }
        }
    }
    OrderPointersStore.erase(p);
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
