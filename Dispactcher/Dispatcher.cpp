#include "../Dispactcher/Dispatcher.hpp"

Dispatcher::~Dispatcher(){
	Close();
}

void Dispatcher::Start() {
	if(started.exchange(true)) return;

	for(int i=0;i<SHARD_COUNT;i++){
		engineTHreads[i] = std::thread(&MatchingEngine::Consumer, &engineArray[i]);
	}

}

void Dispatcher::Close() {

	if (!started.exchange(false)) return;

	for(auto& engine : engineArray){
		engine.Stop();
	}

	for(auto& t : engineTHreads){
		if(t.joinable()){
			t.join();
		}
	}
}


/*
void Dispatcher::PrintAllOrderBooks(){
	for(int i=0;i<SHARD_COUNT;i++){
		std::cout << "Shard count : " << i << std::endl;
		engineArray[i].PrintAllOrderBooks();
		std::cout << std::endl;
	}
}


void Dispatcher::PrintOrderHistory(){
	for(int i=0;i<SHARD_COUNT;i++){
		std::cout << "Shard count : " << i << std::endl;
		engineArray[i].PrintOrderHistory();
		std::cout << std::endl;
	}
}
void MatchingEngine::PrintTrades() {
	for (const auto& trad : trades)
	{
		std::cout << "Buyer id : " << trad.BuyId << " seller ID : " << trad.SellId << " price is: " << trad.price << " quantity is: " << trad.quant << std::endl;
	}
}


void Dispatcher::printTrades() {
	for (auto& t : engineArray) {
		t.PrintTrades();
	}
}
*/
