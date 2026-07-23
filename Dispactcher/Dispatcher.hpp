#pragma once

#include "../CircularQueue/circularQueue.hpp"
#include "../Command.hpp"
#include <unordered_map>
#include "../MatchingEngine/MatchingEngine.hpp"
#include <atomic>

#include<thread>

const int SHARD_COUNT = 6;

class Dispatcher {

	// this will keep record of the symbol mapped to the respective engine instance
	std::thread engineTHreads[SHARD_COUNT];
	std::atomic<bool> started {false};
	std::thread dispatcherThread;
	
	//hashing function for fast hasing of string to random number and bounding that in between shardcount gloabl varibale

	public:
	MatchingEngine engineArray[SHARD_COUNT];
	
	inline size_t hashString(const uint8_t symbol) {
		/*
		uint32_t h = 2166136261;
		for (size_t i = 0; i < symbol ; i++) {
				h ^= uint32_t(symbol);
				h *= 16777619;
		}
		return int(h % uint32_t(SHARD_COUNT));
		*/
		return symbol % SHARD_COUNT;
	}
	Dispatcher() { }
	
	~Dispatcher();

	void printTrades();

	void Start() ;

	void Close();

	void PrintAllOrderBooks();

	void PrintOrderHistory();
	uint64_t ProcessedOrders() const;
};