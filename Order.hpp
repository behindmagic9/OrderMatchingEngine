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
    std::string symbol;
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
    std::string symbol;
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
