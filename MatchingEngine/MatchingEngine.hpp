#pragma once

#include "../Command.hpp"
#include "../Order.hpp"
#include<unordered_map>
#include<vector>
#include<unordered_set>
#include<mutex>
#include<array>
#include<memory>
#include<thread>
#include<atomic>
#include<queue>
#include<iostream>
#include "../CircularQueue/circularQueue.hpp"
#include "../OrderBook/OrderBook.hpp"

class MatchingEngine {
private:
    std::array< OrderBook, 256> orderBook; // sybol assiciated orderbook
    //std::unordered_map<uint64_t, std::vector<OrderEvent>> orderHistory; // orderid, Orders vector    
    std::unordered_set<uint64_t> orderIds;
    CircularQueue<Command, 8192> qe;
    std::mutex mtx;
    bool closed = false;
    std::condition_variable cv;
    //std::unordered_set<int> symbols_set; // its the set of number of symbols entered in system
    //std::vector<Trade> trades;
    std::atomic<bool> stop{false};
    std::atomic<uint64_t> processedOrders{0};

public:

    MatchingEngine();

    void Submit(Command&&);

    void PrintTrades();

    void RecordTrade(Order& incoming, Order& recieving, uint32_t quantity,uint32_t  price);

    template <typename OppositeBook, typename Compare>
    bool CanFullFillOrder(const Order& order, OppositeBook& oppositeBook, Compare comp);

    void Consumer();

    template <typename OppositeBook, typename Compare>
    void MatchOrder(Order& order, OppositeBook& oppositeBook, Compare comp);

    void ProcessOrder(Order& order);
    void RecordOrderEvent(Order& order, Status newStatus, uint32_t execquantity = 0, uint32_t origquantity = -1);
    void PrintOrderHistory();
    void ProcessBUY(Order& order);
    void ProcessSELL(Order& order);
    bool DuplicateOrder(uint64_t orderId);
    void CancelOrder(uint64_t Orderid, uint8_t symbol);//just a function to delegate to orderbook cancel order thing
    void ModifyOrder(uint64_t orderId, uint8_t symbol, uint32_t newprice = -1, uint32_t newquantity = -1, char newside = '\0'); // -1 mean no value thats why by default

    //getters
    OrderBook& GetOrderBook(const uint8_t& symbol) {
        return orderBook[symbol];
    }

    void PrintAllOrderBooks();

    void Stop();

    bool QueueEmpty() const;

    uint64_t ProcessedOrders() const
    {
        return processedOrders.load(std::memory_order_acquire);
    }
};