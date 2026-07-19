#pragma once
#include<cstdint>
#include<string>

#pragma pack(push,1)

struct WireModify {
    uint64_t orderId;
    char symbol[56];
    uint32_t newPrice;
    uint32_t newQuantity;
    char newSide;
};

#pragma pack(pop)