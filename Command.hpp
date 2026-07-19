#pragma once
#include "Order.hpp"
#include<variant>

struct NewOrder
{
    Order order;
    bool fromReplace = false;
};

struct Cancel_Order
{
    uint64_t orderId;
    uint64_t symbol;
};

struct Modify_Order
{
    uint64_t orderId;
    uint64_t newprice = -1;
    uint64_t symbol;
    uint64_t newquantity = -1;
    char newside = '\0';
};

//enum class CommandType
//{
//    New,
//    Modify,
//    Cancel
//};

using CommandData = std::variant< NewOrder ,Cancel_Order ,Modify_Order >;


/*
The command payloads now contain std::string (and Order), which are non-trivial types.
Non-trivial types have constructors, destructors, and potentially custom copy/move operations,
so a plain union cannot manage their lifetime automatically.
Using a union would require manual construction/destruction of the active member (placement new, explicit destructor calls, etc.).
So Replaced the union with std::variant, which safely manages the active object's lifetime internally,
and used std::get_if for type-safe access in the consumer.
*/

// factory fuction for getting commands
struct Command
{
    CommandData data;

    //CommandType type;
    //union
    //{
    //    NewOrder neworder;
    //    CancelOrder cancelOrder;
    //    ModifyOrder modifyOrder;
    //};

    static Command New(const Order& order, bool replace = false)
    {
        return Command{ NewOrder{order, replace } };
    }

    static Command Modify(uint64_t id,uint64_t symbol, uint64_t price = -1, uint64_t quanitity = -1, char side = '\0')
    {
        return Command{ Modify_Order{ id ,price,std::move(symbol), quanitity, side } };
    }

    static Command Cancel(uint64_t id, uint64_t symbol)
    {
        return Command{ Cancel_Order{id,std::move(symbol) } };
    }

    const uint64_t Symbol() const {
        if (auto* n = std::get_if<NewOrder>(&data)) {
            return n->order.symbol;
        }
        else if (auto* n = std::get_if<Cancel_Order>(&data)) {
            return n->symbol;
        }
        else if (auto* n = std::get_if<Modify_Order>(&data) ){
            return n->symbol;
        }
        throw std::logic_error("invalid command");
    }
};