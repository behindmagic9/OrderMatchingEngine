#include "../Network/TCPClient.hpp"
#include<iostream>

TCPClient::TCPClient(const std::string& ip , uint16_t port) : _ip(ip), _port(port){

}

bool TCPClient::Connect(){
    _socket = socket(AF_INET , SOCK_STREAM,0);

    if(_socket < 0){
        return false;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(_port);

    inet_pton(AF_INET, _ip.c_str(), &server.sin_addr);

    if(connect(_socket,reinterpret_cast<sockaddr*> (&server) , sizeof(server)) < 0){
        return false;
    }

    connected = true;
    return true;
}

void TCPClient::Disconnect(){
    if(!connected){
        return ;
    }

    close(_socket);
    connected = false;
}

bool TCPClient::Send(const std::vector<char>& data){
    return Send(data.data(), data.size());
}

bool TCPClient::Send(const void* data, size_t size){
    const char* ptr = static_cast<const char*> (data);

    while(size > 0){
        int sent = send(_socket,ptr ,static_cast<int>(size) , 0);
        if(sent <= 0){
            return false;
        }

        ptr += sent;
        size -= sent;
    }

    return true;
}