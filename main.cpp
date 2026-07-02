#include<iostream>
#include<queue>
#include<map>
#include<algorithm>
#include<vector>
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

std::vector<Trade> trades;
std::queue<Order> qe;

std::map<int, std::queue<Order> , std::greater<int>> BUY;
std::map<int, std::queue<Order>> SELL;

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
	    RecordTrade(order, ord, quant, it->first);

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
            // trade is heppenig so will record trade obejct here 
	    RecordTrade(order, ord, quant, it->first);
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
        std::cout << "partial filled remain saved to orderbook" << std::endl;
        SELL[order.price].push(order);
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
    }
    else {
        // nothgin dropp it jsut , not meainngful for us
	std::cout << "invalid side given " << std::endl;
    }
}

void PrintTrades(){
        for(int i =0;i<trades.size();i++){
        	std::cout << "Buyer id : " << trades[i].BuyId <<  " seller ID : " << trades[i].SellId << " price is: " << trades[i].price << " quantity is: " << trades[i].quant << std::endl;
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

void PrintOrderBook(){
	std::cout << "BUY\n";
	for(auto &level : BUY){
		int total=0;
		auto q = level.second;
		while(!q.empty())
		{
			total+= q.front().quantity;
			q.pop();
		}
		std::cout << "price " <<level.first << " quantity " << total << "\n";
	}
	
	std::cout << "SELL\n";
	for(auto &level:SELL){
		int total =0;
		auto q = level.second;
		while(!q.empty()){
			total += q.front().quantity;
			q.pop();
		}
		std::cout << "price " <<level.first << " quantity " << total << "\n";
	}
}

int main() {
    Producer();
    Consumer();
    PrintTrades();
    PrintOrderBook();
    return 0;
}
