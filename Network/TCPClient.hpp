#pragma once

#include <cstdint>
#include<arpa/inet.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include<unistd.h>

class TCPClient{

    public:
        TCPClient(const std::string& ip , uint16_t port);
        bool Connect();
        void Disconnect();

        bool Send(const void* data , size_t size);
        bool Send(const std::vector<char>& data);

    private:    
        std::string _ip;
        uint16_t _port;
        int _socket = -1;

        bool connected = false;
};

