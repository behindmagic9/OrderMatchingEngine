#pragma once
#include <cstdint>
#include<arpa/inet.h>
#include <unistd.h>
#include<atomic>
#include<vector>
#include "../Network/Encode/Deserializer.hpp"
#include "../Dispactcher/Dispatcher.hpp"

class TCPServer{
    int serverFD = -1;
    uint16_t port;
    Dispatcher& dispatcher;
    std::atomic <bool> running;
    std::thread acceptThread;
    std::vector<std::thread> clientThread;
    Deserializer& _deserializer;

    void Accept();
    void ClientLoop(int clientFd);
    void Stop();
    public:
    TCPServer(uint16_t p , Dispatcher& d, Deserializer& ds) : port(p), dispatcher(d) , _deserializer(ds){
        
    }

    ~TCPServer(){
        Stop();
    }


    bool Start();


};