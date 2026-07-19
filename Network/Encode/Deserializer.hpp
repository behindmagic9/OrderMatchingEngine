#pragma once

#include<cstring>
#include<atomic>
#include<iostream>
#include "../../Command.hpp"
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
                std::string symbol(order.symbol);
                uint64_t orId = orderId.fetch_add(1);
                auto id = _registry.GetSymbolId(symbol);
                if(!id){
                    throw std::runtime_error("unkonw symbol");
                }
                return Command::New({orId, order.side, order.price, order.quantity, id.value(), static_cast<OrderType>(order.orderType), static_cast<OrderTimeinFrame>(order.tif)});
            }
            case PacketTpe::CancelOrder:
            {
                WireCancel order;
                std::memcpy(&order, body.data(), sizeof(order)); 
                std::string symbol(order.symbol);
                auto id = _registry.GetSymbolId(symbol);
                if(!id){
                    throw std::runtime_error("unkonw symbol");
                }
                return Command::Cancel(order.orderId, id.value());
            }
            case PacketTpe::ModifyOrder: 
            {
                WireModify order;
                std::memcpy(&order, body.data(), sizeof(order));
                std::string symbol(order.symbol);
                uint64_t orId = orderId.fetch_add(1);
                auto id = _registry.GetSymbolId(symbol);
                return Command::Modify(orId, id.value(),order.newPrice, order.newQuantity, order.newSide);
            }
            default:
                throw std::runtime_error("ubknown pacet");
        }
    }

    private:
    SymbolRegistry& _registry;
    std::atomic<uint64_t> orderId {1};
};