#pragma once
#include<cstdint>
#include<string>

#pragma pack(push,1)

struct WireCancel {
    uint64_t orderId;
    char symbol[56];
};

#pragma pack(pop)