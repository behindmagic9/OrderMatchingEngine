#pragma once

#include "../Command.hpp"
#include "../Order.hpp"
#include<unordered_map>
#include<vector>
#include<unordered_set>
#include<queue>
#include<iostream>
#include "../OrderBook/OrderBook.hpp"

class MatchingEngine {
private:
    OrderBook orderBook;
    std::unordered_map<int, std::vector<OrderEvent>> orderHistory; // orderid, Orders vector
    std::unordered_set<int> orderIds;
    std::queue<Command> qe;

public:

    void Submit(const Command&);

    template <typename OppositeBook, typename Compare>
    bool CanFullFillOrder(const Order& order, OppositeBook& oppositeBook, Compare comp);

    void Consumer();

    template <typename OppositeBook, typename Compare>
    void MatchOrder(Order& order, OppositeBook& oppositeBook, Compare comp);

    void ProcessOrder(Order& order);
    void RecordOrderEvent(Order& order, Status newStatus, int execquantity = 0, int origquantity = -1);
    void PrintOrderHistory();
    void ProcessBUY(Order& order);
    void ProcessSELL(Order& order);
    bool DuplicateOrder(int orderId);
    void CancelOrder(int Orderid);//just a function to delegate to orderbook cancel order thing
    void ModifyOrder(int orderId, int64_t newprice = -1, int newquantity = -1, char newside = '\0'); // -1 mean no value thats why by default

    //getters

    OrderBook& GetOrderBook() {
        return orderBook;
    }

};