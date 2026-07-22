#include "OrderBook.hpp"

void OrderBook::AddToOrderBook(Order&& order) {
    Order *p = new Order(std::move(order)); // making a heap copy cause now using the intrusive list 
    if (p->side == 'B') {

        BUY[p->price].push_back(*p);
        auto it = BUY[p->price].iterator_to(*p);
        OrderPointersStore[p->orderId] = { p->price,p->side,it };
    }
    else {
        SELL[p->price].push_back(*p);
        auto it = SELL[p->price].iterator_to(*p);
        OrderPointersStore[p->orderId] = { p->price,p->side,it };
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
    Order *order = &(*it.iterator);
    Order cancelledOrder = *(order);
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
    delete order;
    return cancelledOrder;
}


OrderBook::OrderBook(){
    OrderPointersStore.reserve(4096);
}
