#include "../Dispactcher/Dispatcher.hpp"

void Dispatcher::Dispatching() {
	while(true){
		auto cmd = GlobalQueue.pop();
		
		if(!cmd){
			break;
		}

		engineArray[hashString(cmd->Symbol())].Submit(std::move(*cmd));
	}

	for(auto& engine : engineArray){
		engine.CloseQueue();
	}
}

Dispatcher::~Dispatcher(){
	Close();
}

void Dispatcher::Start() {
	if(started.exchange(true)) return;

	for(int i=0;i<SHARD_COUNT;i++){
		engineTHreads[i] = std::thread(&MatchingEngine::Consumer, &engineArray[i]);
	}

	dispatcherThread = std::thread(&Dispatcher::Dispatching, this);
}

void Dispatcher::submit(Command&& cmd) {
	GlobalQueue.push(std::move(cmd));
}

void Dispatcher::Close() {

	if (!started.exchange(false)) return;
	GlobalQueue.close();

	if(dispatcherThread.joinable()){
		dispatcherThread.join();
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
