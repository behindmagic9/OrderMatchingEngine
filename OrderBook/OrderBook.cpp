#include "OrderBook.hpp"

void OrderBook::AddToOrderBook(const Order& order) {
    if (order.side == 'B') {
        BUY[order.price].push_back(order);
        auto it = std::prev(BUY[order.price].end());
        OrderPointersStore[order.orderId] = { order.price,order.side,it };
    }
    else {
        SELL[order.price].push_back(order);
        auto it = std::prev(SELL[order.price].end());
        OrderPointersStore[order.orderId] = { order.price,order.side,it };
    }
}


void OrderBook::PrintOrderBook() {
    std::cout << "BUY  >> ";

    for (const auto& p : BUY)
    {
        int total = 0;
        for (const auto& ord : p.second)
        {
            total += ord.quantity;
        }
        std::cout << "price : " << p.first << " quanitity " << total << std::endl;
    }

    std::cout << "SELL >> ";

    for (const auto& p : SELL)
    {
        int total = 0;
        for (const auto& ord : p.second)
        {
            total += ord.quantity;
        }
        std::cout << "price : " << p.first << " quanitity " << total << std::endl;
    }
    std::cout << std::endl;
}

std::optional<Order> OrderBook::CancelOrder(uint64_t Orderid) {
    auto p = OrderPointersStore.find(Orderid);
    if (p == OrderPointersStore.end()) {
        return std::nullopt;
    }
    auto it = p->second;
    Order cancelledOrder = *(it.iterator);
    if (it.side == 'B') {
        auto lst = BUY.find(it.price);
        if (lst != BUY.end()) {
            lst->second.erase(it.iterator);
            if (lst->second.empty()) {
                BUY.erase(lst);
            }
        }
    }
    else {
        auto lst = SELL.find(it.price);
        if (lst != SELL.end()) {
            lst->second.erase(it.iterator);
            if (lst->second.empty()) {
                SELL.erase(lst);
            }
        }
    }
    OrderPointersStore.erase(p);
    return cancelledOrder;
}


OrderBook::OrderBook(){
    OrderPointersStore.reserve(1026);
}
