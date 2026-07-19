#pragma once
#include<cstdint>
#include<string>

#pragma pack(push,1)

struct WireNewOrder{
    uint64_t orderId;
    char symbol[56];
    uint32_t price;
    uint32_t quantity;
    char side;
    uint8_t orderType;
    uint8_t tif;
};

#pragma pack(pop)

