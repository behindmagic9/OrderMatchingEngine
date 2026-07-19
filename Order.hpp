#pragma once

#include<list>
#include<string>
#include<cstdint>
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

enum class OrderType : uint8_t
{
    Limit,
    Market
};

enum class OrderTimeinFrame : uint8_t
{
    GTC, // Good Till Cancel
    FOK, // Fill or KILL
    IOC  // Immediate or KIll
};

struct Order {
    uint64_t orderId;
    char side;
    uint32_t price;
    uint32_t quantity;
    uint8_t symbol;
    OrderType otype;
    OrderTimeinFrame otf;
    Status status;
};

struct Trade {
    uint64_t BuyId;
    uint64_t SellId;
    uint32_t quant;
    uint32_t price;
};


struct OrderEvent {
    Status oldStatus;
    Status newStatus;
    uint32_t price;
    uint8_t symbol;
    uint32_t originalquantity;
    uint32_t execquantity;
    uint32_t remquantity;
};

struct OrderRef {
    uint32_t price;
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
