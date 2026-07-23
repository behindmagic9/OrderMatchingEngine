#pragma once
#include<unordered_map>
#include<map>
#include"../Order.hpp"
#include<iostream>
#include"../Command.hpp"
#include <optional>
#include<boost/intrusive/list.hpp>

class OrderBook {
private:
    using OrderList = bi::list<Order>;
    std::map<uint64_t, OrderList, std::greater<uint64_t>> BUY;
    std::map<uint64_t, OrderList> SELL;
    OrderPool pool;
public:
    std::unordered_map<uint64_t, OrderRef> OrderPointersStore; // orderid, iterator
    void AddToOrderBook(Order&& order);
    OrderBook();
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

    void ReleaseOrder(Order* order){
        pool.release(order);
    }
};

