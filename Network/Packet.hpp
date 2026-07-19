#pragma once 

#include <cstdint>

#include "../Network/Wire/NewOrderWire.hpp"
#include "../Network/Wire/WireCancel.hpp"
#include "../Network/Wire/WireModify.hpp"

enum class PacketTpe : uint32_t{
    NewOrder = 1,
    CancelOrder = 2,
    ModifyOrder = 3,
};

struct PacketHeader{
    uint32_t length;
    PacketTpe type;
};

struct NewOrderPacket
{
    PacketHeader newPacket;
    WireNewOrder body;
};

struct CancelPacket
{
    PacketHeader header;
    WireCancel cancelBody;
};

struct ModifyPacket
{
    PacketHeader header;
    WireModify modifyBody;
};