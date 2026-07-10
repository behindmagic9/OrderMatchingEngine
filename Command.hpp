#pragma once
#include "Order.hpp"

struct NewOrder
{
    Order order;
    bool fromReplace = false;
};

struct CancelOrder
{
    int orderId;
};

struct ModifyOrder
{
    int orderId;
    int64_t newprice = -1;
    int newquantity = -1;
    char newside = '\0';
};

enum class CommandType
{
    New,
    Modify,
    Cancel
};

struct Command
{
    CommandType type;

    union
    {
        NewOrder neworder;
        CancelOrder cancelOrder;
        ModifyOrder modifyOrder;
    };

    static Command New(const Order& order, bool replace = false)
    {
        Command c;
        c.type = CommandType::New;
        c.neworder = { order,replace };
        return c;
    }

    static Command Modify(int id, int64_t price = -1, int quanitity = -1, char side = '\0')
    {
        Command c;
        c.type = CommandType::Modify;
        c.modifyOrder = { id, price, quanitity, side };
        return c;
    }

    static Command Cancel(int id)
    {
        Command c;
        c.type = CommandType::Cancel;
        c.cancelOrder = { id };
        return c;
    }

    Command() {}
    ~Command() {}
};

