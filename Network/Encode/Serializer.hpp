#pragma once
#include<vector>
#include<cstring>
#include "../../Network/Packet.hpp"

class Serializer{
    public:
        template<typename T>
        static std::vector<char> Serialize(PacketTpe type, const T& body){
            
            PacketHeader header{};

            header.type = type;
            header.length = htonl(sizeof(T));
            
            std::vector<char> packet(sizeof(PacketHeader) + sizeof(T));

            std::memcpy(packet.data(), &header, sizeof(PacketHeader)); // dest , soruce  size of T
            std::memcpy(packet.data()+sizeof(PacketHeader), &body, sizeof(T)); // dest , soruce  size of T

            return packet;
        }
};

