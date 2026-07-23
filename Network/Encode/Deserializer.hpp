#pragma once

#include<cstring>
#include<atomic>
#include<iostream>
#include "../../Command.hpp"
#include<array>
#include "../../Network/Packet.hpp"
#include "../../Helper/SymbolRegistry/SymbolRegistry.hpp"

class Deserializer
{
    public:
    Deserializer(SymbolRegistry& registry) : _registry(registry){

    }
    Command Deserialize(const PacketHeader& header,const std::vector<char>& body){

        switch(header.type){
            case PacketTpe::NewOrder:
            {
                WireNewOrder order;
                std::memcpy(&order, body.data(), sizeof(order));
                std::string_view symbol(order.symbol);
                uint64_t orId = orderId.fetch_add(1);
                auto symid = _registry.GetSymbolId(symbol);
                if(!symid){
                    throw std::runtime_error("unkonw symbol");
                }
                OrderData ord{};
                ord.orderId = orId;
                ord.side = order.side;
                ord.price = order.price;
                ord.quantity = order.quantity;
                ord.symbol = symid.value();
                ord.otype = static_cast<OrderType>(order.orderType);
                ord.otf = static_cast<OrderTimeinFrame>(order.tif);
                ord.status = Status::NEW;
                return Command::New(ord);
                //return Command::New({orId, order.side, order.price, order.quantity, symid.value(), static_cast<OrderType>(order.orderType), static_cast<OrderTimeinFrame>(order.tif)});
            }
            case PacketTpe::CancelOrder:
            {
                WireCancel order;
                std::memcpy(&order, body.data(), sizeof(order)); 
                std::string_view symbol(order.symbol);
                auto symid = _registry.GetSymbolId(symbol);
                if(!symid){
                    throw std::runtime_error("unkonw symbol");
                }
                return Command::Cancel(order.orderId, symid.value());
            }
            case PacketTpe::ModifyOrder: 
            {
                WireModify order;
                std::memcpy(&order, body.data(), sizeof(order));
                std::string_view symbol(order.symbol);
                auto symid = _registry.GetSymbolId(symbol);
                return Command::Modify(order.orderId, symid.value(),order.newPrice, order.newQuantity, order.newSide);
            }
            default:
                throw std::runtime_error("ubknown pacet");
        }
    }

    private:
    SymbolRegistry& _registry;
    std::atomic<uint64_t> orderId {1};
};