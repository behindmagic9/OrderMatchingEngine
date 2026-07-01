#include<iostream>
#include<queue>
#include<map>
#include<algorithm>
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
    int SELLId;
    int BUYID;
    int quant;
    int price;
};

std::queue<Order> qe;

std::map<int, std::queue<Order> , std::greater<int>> BUY;
std::map<int, std::queue<Order>> SELL;

void Producer() {
    for (int i = 0; i < 5; i++) {
        qe.push(Order{ i,'B',i * 4, 35 });
        qe.push(Order{ i,'S',i * 2, 20 });
        qe.push(Order{ i,'B',i * 4, 30 });
        qe.push(Order{ i,'S',i * 2, 25 });
    }
}

void ProcessBUY(Order order) {
    auto it = SELL.begin();
    while (order.quantity > 0 && it != SELL.end()) {
        if (it->first > order.price) {
            // break when sell price is greater than order buy price cause , terminate condition is (sell > buy)
            // will help in optimization too not loop in map ,and directly come to conclusion
            // and also help in breaking while LOOP
            break;
        }
        auto& q = it->second;
        while (order.quantity > 0 && !q.empty()) {
            auto& ord = q.front();
            int quant = std::min(ord.quantity, order.quantity);

            // trade is heppenig so will record trade obejct here 

            order.quantity -= quant;
            ord.quantity -= quant;
            if (ord.quantity == 0) {
                it->second.pop();
            }
        }
        if (q.empty()) {
            it = SELL.erase(it);
        }
        else {
            it++;
        }
        // look into the queue and take the first elem adn match the trade with that
        // Order temp = it->second.front();
        // here ahvet o subtrat this from the quqnity til order complete
        // or just do while(!(order.price < it->first && order.quantity <= 0)){} 
        //in mid return if quantity is done , then return the TRade struct
    }
    if (order.quantity > 0) {
        // marking this as partial filled
        std::cout << "partial filled buy" << std::endl;
        BUY[order.price].push(order);
    }
    else {
        std::cout << "trade successfull in buy" << std::endl;
        // order executed successfully
    }
}

void ProcessSELL(Order order) {
    auto it = BUY.begin();
    while (order.quantity > 0 && it != BUY.end()) {
        if (it->first < order.price) {
            // break when sell price is greater than order buy price cause , terminate condition is (sell > buy)
            // will help in optimization too not loop in map ,and directly come to conclusion
            // and also help in breaking while LOOP
            break;
        }
        auto& q = it->second;
        while (order.quantity > 0 && !q.empty()) {
            auto& ord = q.front();
            int quant = std::min(ord.quantity, order.quantity);
            order.quantity -= quant;
            ord.quantity -= quant;
            if (ord.quantity == 0) {
                it->second.pop();
            }
        }
        if (q.empty()) {
            it = BUY.erase(it);
        }
        else {
            it++;
        }
        // look into the queue and take the first elem adn match the trade with that
        // Order temp = it->second.front();
        // here ahvet o subtrat this from the quqnity til order complete
        // or just do while(!(order.price < it->first && order.quantity <= 0)){} 
        //in mid return if quantity is done , then return the TRade struct
    }
    if (order.quantity > 0) {
        // marking this as partial filled
        std::cout << "partial filled sell" << std::endl;
        SELL[order.price].push(order);
    }
    else {
        std::cout << "trade successfull in sell" << std::endl;
        // order executed successfully
    }
}

void ProcessOrder(Order order) {
    if (order.side == 'B') {
        // first have to see if the map is null or not
        // if null then inet directly
        if (SELL.empty()) {
            std::cout << "directly pushed to buy" << std::endl;
            BUY[order.price].push(order);
        }
        else {
            ProcessBUY(order);
        }
    }
    else if (order.side == 'S') {
        if (BUY.empty()) {
            std::cout << "directly pushed to sell" << std::endl;
            SELL[order.price].push(order);
        }
        else {
            ProcessSELL(order);
        }
        // if nothign in that order queue just insert directly in that mean store in same order book .. look in opp and push in same type
        // if already present there then look into that order queue and see that back of that queue, if that is less than or equal to the BUY price
        // if that is then execute that trade and iterate from reverse into the queue and go until trade is executng and quanitty does not get zero
        // look into the SELL map and see if it can match the
        // lower sell side
        // if yes substract the quantity until it wont trade condition
        // then store in orderbook of BUY
        //Order temp = BUY[order.price].front();
        // same like above
        // if not found will store in SELL orderbook
    }
    else {
        // nothgin dropp it jsut , not meainngful for us
    }
}

void Consumer() {
    while (!qe.empty()) {
        Order temp = qe.front();
        qe.pop();
        ProcessOrder(temp);
       // std::cout << "order id : " << temp.orderId << "side : " << temp.side << " price: " << temp.price << std::endl;
    }
}

int main() {
    Producer();
    Consumer();
    return 0;
}
