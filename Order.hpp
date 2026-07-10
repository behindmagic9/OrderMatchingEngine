#pragma once

#include<list>
#include<string>
#include<iostream>
#include<vector>
enum class Status {
    NEW,
    OPEN,
    FILLED,
    PARTIAL_FILLED,
    PARTIAL_FILLED_Cancel,
    CANCELLED,
    REJECTED
};

enum class OrderType
{
    Limit,
    Market
};

enum class OrderTimeinFrame
{
    GTC, // Good Till Cancel
    FOK, // Fill or KILL
    IOC  // Immediate or KIll
};

struct Order {
    int orderId;
    char side;
    int64_t price;
    int quantity;
    OrderType otype;
    OrderTimeinFrame otf;
    Status status;
};

struct Trade {
    int BuyId;
    int SellId;
    int quant;
    int64_t price;
};


struct OrderEvent {
    Status oldStatus;
    Status newStatus;
    int64_t price;
    int originalquantity;
    int execquantity;
    int remquantity;
};

struct OrderRef {
    int64_t price;
    char side;
    std::list<Order>::iterator iterator;
};

inline bool Validator(const Order& order) {
    if (order.orderId <= 0) return false;
    if (order.quantity <= 0) return false;
    if (order.side != 'B' && order.side != 'S') return false;
    if (order.otype == OrderType::Limit && order.price <= 0) return false;
    if (order.otype == OrderType::Market && order.otf == OrderTimeinFrame::GTC) return false;
    return true;
}

inline std::string StatusToString(Status status)
{
    switch (status)
    {
    case Status::NEW:
        return "NEW";
    case Status::OPEN:
        return "OPEN";
    case Status::FILLED:
        return "FILLED";
    case Status::PARTIAL_FILLED:
        return "PARTIAL_FILLED";
    case Status::PARTIAL_FILLED_Cancel:
        return "PARTIAL_FILLED_CANCEL";
    case Status::CANCELLED:
        return "CANCELLED";
    case Status::REJECTED:
        return "REJECTED";
    default:
        return "UNKNOWN";
    }
}

/*
Order.hpp is included in several .cpp files. Since it contains the definition std::vector<Trade> trades;,
each .cpp creates its own global vector named trades. The compiler compiles each file independently, so it does not complain. 
Later the linker combines the object files and finds multiple global definitions with the same name, which violates the One Definition Rule and causes LNK2005.
#pragma once only prevents duplicate inclusion inside one .cpp; it does not prevent the header from being included by different .cpp files. inline makes identical header definitions legal and
merges them into one shared variable in C++17.
*/
inline std::vector<Trade> trades;

inline void PrintTrades() {
    for (const auto& trad : trades)
    {
        std::cout << "Buyer id : " << trad.BuyId << " seller ID : " << trad.SellId << " price is: " << trad.price << " quantity is: " << trad.quant << std::endl;
    }
}

inline void RecordTrade(Order& incoming, Order& recieving, int quantity, int64_t price) {
    Trade t{
        incoming.side == 'B' ? incoming.orderId : recieving.orderId,
        incoming.side == 'S' ? incoming.orderId : recieving.orderId,
        quantity,
        price };

    trades.push_back(t);
}
