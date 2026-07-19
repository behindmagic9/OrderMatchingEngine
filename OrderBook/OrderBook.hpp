#pragma once
#include<unordered_map>
#include<map>
#include"../Order.hpp"
#include<iostream>
#include"../Command.hpp"
#include <optional>

class OrderBook {
private:

    std::map<uint64_t, std::list<Order>, std::greater<uint64_t>> BUY;
    std::map<uint64_t, std::list<Order>> SELL;
public:
    std::unordered_map<uint64_t, OrderRef> OrderPointersStore; // orderid, iterator
    void AddToOrderBook(const Order& order);

    // std::optional is used to represent an object that whether it contain a value or not, so here it either return a order or return nulloptional
    std::optional<Order> CancelOrder(uint64_t Orderid);
    void PrintOrderBook();

    //Getters
    auto& GetBuyBook() {
        return BUY;
    }
    auto& GetSellBook() {
        return SELL;
    }

    void RemovePointer(uint64_t id) {
        OrderPointersStore.erase(id);
    }
};

