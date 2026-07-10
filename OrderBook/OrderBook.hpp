#pragma once
#include<unordered_map>
#include<map>
#include"../Order.hpp"
#include<iostream>
#include"../Command.hpp"
#include <optional>

class OrderBook {
private:

    std::map<int64_t, std::list<Order>, std::greater<int64_t>> BUY;
    std::map<int64_t, std::list<Order>> SELL;
public:
    std::unordered_map<int, OrderRef> OrderPointersStore; // orderid, iterator
    void AddToOrderBook(const Order& order);

    // std::optional is used to represent an object that whether it contain a value or not, so here it either return a order or return nulloptional
    std::optional<Order> CancelOrder(int Orderid);
    void PrintOrderBook();

    //Getters
    auto& GetBuyBook() {
        return BUY;
    }
    auto& GetSellBook() {
        return SELL;
    }

    void RemovePointer(int id) {
        OrderPointersStore.erase(id);
    }

    bool OrderPointerFound(int id) {
        return OrderPointersStore.find(id) == OrderPointersStore.end();
    }
};

