#include "../Network/TCPClient.hpp"
#include"../Order.hpp"
#include "../Network/Wire/NewOrderWire.hpp"
#include "../Network/Encode/Serializer.hpp"
#include<string>

int main(){
    TCPClient client("127.0.0.1", 2889);

    if(!client.Connect()){
        std::cout << "not able to connect to client" << std::endl;
        return -1;
    }

    WireNewOrder order{};

    std::strncpy(order.symbol, "BTC", sizeof(order.symbol)-1);
    order.price=100;
    order.side='B';
    order.quantity=50;
    order.orderType= static_cast<uint8_t>(OrderType::Limit);
    order.tif = static_cast<uint8_t>(OrderTimeinFrame::GTC);

    // sreialize
    Serializer serializer;
    std::cout << "side: "<< order.side << std::endl;
    auto packet = serializer.Serialize(PacketTpe::NewOrder, order);
    
    if(client.Send(packet)){
        std::cout << "order snet successfully" << std::endl;
    }else{
        std::cout << "failed to send order" << std::endl;
    }
    client.Disconnect();
    return 0;
}

/*
g++ -o3 -DNDEBUG -std=c++17 -pthread Client/stressmain.cpp Network/TCPClient.cpp -I. -o client
*/