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
    int orderId;
    std::string symbol;
};

struct Modify_Order
{
    int orderId;
    int64_t newprice = -1;
    std::string symbol;
    int newquantity = -1;
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

    static Command Modify(int id,std::string symbol, int64_t price = -1, int quanitity = -1, char side = '\0')
    {
        return Command{ Modify_Order{ id ,price,std::move(symbol), quanitity, side } };
    }

    static Command Cancel(int id, std::string symbol)
    {
        return Command{ Cancel_Order{id,std::move(symbol) } };
    }
};
