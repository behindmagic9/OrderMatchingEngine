#pragma once

#include "../ThreadSafeQueue/TQueue.hpp"
#include "../Command.hpp"
#include <unordered_map>
#include "../MatchingEngine/MatchingEngine.hpp"
#include <atomic>
#include<thread>

const int SHARD_COUNT = 6;

class Dispatcher {
	// this will the be the one to which initial commands are to be published or to be stored 
	TQueue<Command> GlobalQueue;

	// this will keep record of the symbol mapped to the respective engine instance
	MatchingEngine engineArray[SHARD_COUNT];
	std::thread engineTHreads[SHARD_COUNT];
	std::atomic<bool> started {false};
	std::thread dispatcherThread;
	
	//hashing function for fast hasing of string to random number and bounding that in between shardcount gloabl varibale
	size_t hashString(const std::string& symbol) {
		uint32_t h = 2166136261;
		for (size_t i = 0; i < symbol.length() ; i++) {
				h ^= uint32_t(symbol[i]);
				h *= 16777619;
		}
		return int(h % uint32_t(SHARD_COUNT));
	}

	void Dispatching() ;

	public:

	Dispatcher() { }
	
	~Dispatcher();

	void printTrades();

	void Start() ;

	void submit(Command&& cmd);

	void Close();

	void PrintAllOrderBooks();

	void PrintOrderHistory();
};