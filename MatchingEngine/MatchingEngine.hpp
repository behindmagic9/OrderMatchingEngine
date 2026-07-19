#pragma once

#include "../Command.hpp"
#include "../Order.hpp"
#include<unordered_map>
#include<vector>
#include<unordered_set>
#include<mutex>
#include<queue>
#include<condition_variable>
#include<iostream>
#include "../ThreadSafeQueue/TQueue.hpp"
#include "../OrderBook/OrderBook.hpp"

class MatchingEngine {
private:
    std::unordered_map<uint64_t , OrderBook> orderBook; // sybol assiciated orderbook
    //std::unordered_map<uint64_t, std::vector<OrderEvent>> orderHistory; // orderid, Orders vector    
    std::unordered_set<uint64_t> orderIds;
    TQueue<Command> qe;
    std::mutex mtx;
    bool closed = false;
    std::condition_variable cv;
    //std::unordered_set<int> symbols_set; // its the set of number of symbols entered in system
    //std::vector<Trade> trades;
public:

    void Submit(Command&&);

    void PrintTrades();

    void RecordTrade(Order& incoming, Order& recieving, uint64_t quantity, uint64_t price);

    template <typename OppositeBook, typename Compare>
    bool CanFullFillOrder(const Order& order, OppositeBook& oppositeBook, Compare comp);

    void Consumer();

    template <typename OppositeBook, typename Compare>
    void MatchOrder(Order& order, OppositeBook& oppositeBook, Compare comp);

    void ProcessOrder(Order& order);
    void RecordOrderEvent(Order& order, Status newStatus, uint64_t execquantity = 0, uint64_t origquantity = -1);
    void PrintOrderHistory();
    void ProcessBUY(Order& order);
    void ProcessSELL(Order& order);
    bool DuplicateOrder(int orderId);
    void CancelOrder(uint64_t Orderid, uint64_t symbol);//just a function to delegate to orderbook cancel order thing
    void ModifyOrder(uint64_t orderId, uint64_t symbol, uint64_t newprice = -1, uint64_t newquantity = -1, char newside = '\0'); // -1 mean no value thats why by default

    //getters
    OrderBook& GetOrderBook(const uint64_t& symbol) {
        return orderBook[symbol];
    }

    void PrintAllOrderBooks();

    void CloseQueue();

};