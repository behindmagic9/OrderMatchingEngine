#include "OrderBook.hpp"

void OrderBook::AddToOrderBook(Order&& order) {
    Order *p = pool.acquire(); // making a heap copy cause now using the intrusive list 
    p->data = std::move(order.data);
    if (p->data.side == 'B') {
        uint32_t price = p->data.price;
        BUY[price].push_back(*p);
        auto it = BUY[price].iterator_to(*p);
        OrderPointersStore[p->data.orderId] = { p->data.price,p->data.side,it };
    }
    else {
        uint32_t price = p->data.price;
        SELL[price].push_back(*p);
        auto it = SELL[price].iterator_to(*p);
        OrderPointersStore[p->data.orderId] = { p->data.price,p->data.side,it };
    }
}

void OrderBook::PrintOrderBook() {
    std::cout << "BUY  >> ";

    for (const auto& p : BUY)
    {
        uint32_t total = 0;
        for (const auto& ord : p.second)
        {
            total += ord.data.quantity;
        }
        std::cout << "price : " << p.first << " quanitity " << total << std::endl;
    }

    std::cout << "SELL >> ";

    for (const auto& p : SELL)
    {
        uint32_t total = 0;
        for (const auto& ord : p.second)
        {
            total += ord.data.quantity;
        }
        std::cout << "price : " << p.first << " quanitity " << total << std::endl;
    }
    std::cout << std::endl;
}

std::optional<OrderData> OrderBook::CancelOrder(uint64_t Orderid) {
    auto p = OrderPointersStore.find(Orderid);
    if (p == OrderPointersStore.end()) {
        return std::nullopt;
    }
    auto it = p->second;
    Order *order = &(*it.iterator);
    OrderData cancelorder = order->data;
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
    pool.release(order);
    return cancelorder;
}


OrderBook::OrderBook(){
    OrderPointersStore.reserve(4096);
}
