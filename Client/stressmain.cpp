#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <random>
#include <chrono>
#include <cstring>
#include <atomic>

#include "../Network/TCPClient.hpp"
#include "../Order.hpp"
#include "../Network/Encode/Serializer.hpp"
#include "../Network/Packet.hpp"
#include "../Network/Wire/NewOrderWire.hpp"
#include "../Network/Wire/WireCancel.hpp"
#include "../Network/Wire/WireModify.hpp"

struct OrderInfo
{
    uint64_t id;
    std::string symbol;
    uint64_t price;
    uint32_t qty;
    char side;
};

class TraderClient
{
public:

    TraderClient(int id,const std::string& ip,uint16_t port,uint64_t ordersToSend)
        : client(ip,port),traderId(id),totalOrders(ordersToSend),rng(std::random_device{}())
    {
    }

    void Run()
    {
        if(!client.Connect())
        {
            //std::cout<<"Client "<<traderId<<" failed\n";
            return;
        }

        for(uint64_t i=0;i<totalOrders;i++)
        {
            int action=random(0,99);

            if(activeOrders.empty()) action=0;

            if(action<60)
            {
                SendNew();
            }
            else if(action<85)
            {
                SendModify();
            }
            else
            {
                SendCancel();
            }

            std::this_thread::sleep_for(std::chrono::microseconds(random(20,300)));
        }

        client.Disconnect();
    }

private:

    TCPClient client;

    int traderId;

    uint64_t totalOrders;

    std::mt19937 rng;

    uint64_t nextOrderId=1;

    std::unordered_map<uint64_t,OrderInfo> activeOrders;

    std::vector<std::string> symbols=
    {
        "BTC",
        "ETH",
        "AMD",
        "AAPL",
        "TSLA",
        "GOOG",
        "NVDA",
        "MSFT"
    };

    int random(int l,int r)
    {
        std::uniform_int_distribution<int> dist(l,r);
        return dist(rng);
    }

    char RandomSide()
    {
        return random(0,1)==0 ? 'B' : 'S';
    }

    OrderType RandomType()
    {
        return random(0,9)<8
                ? OrderType::Limit
                : OrderType::Market;
    }

    OrderTimeinFrame RandomTIF()
    {
        int x=random(0,2);

        if(x==0) return OrderTimeinFrame::GTC;

        if(x==1) return OrderTimeinFrame::IOC;

        return OrderTimeinFrame::FOK;
    }

    std::string RandomSymbol()
    {
        return symbols[random(0,symbols.size()-1)];
    }

    uint64_t RandomPrice()
    {
        return random(90,200);
    }

    uint32_t RandomQty()
    {
        return random(1,500);
    }

    auto RandomExisting()
    {
        auto it=activeOrders.begin();

        std::advance(it,random(0,activeOrders.size()-1));

        return it;
    }

    void SendNew()
    {
        WireNewOrder body{};

        uint64_t id=nextOrderId++;

        std::string sym=RandomSymbol();

        body.orderId=id;

        std::memset(body.symbol,0,sizeof(body.symbol));

        std::memcpy(body.symbol,sym.c_str(),sym.size());

        body.price=RandomPrice();

        body.quantity=RandomQty();

        body.side=RandomSide();

        body.orderType= static_cast<uint8_t>(RandomType());

        body.tif= static_cast<uint8_t>(RandomTIF());

        auto packet=
            Serializer::Serialize(
                PacketTpe::NewOrder,
                body
            );

        client.Send(packet);

        activeOrders[id]=
        {
            id,
            sym,
            body.price,
            body.quantity,
            body.side
        };
    }

    void SendModify()
    {
        if(activeOrders.empty())return;

        auto it = RandomExisting();

        WireModify body{};

        body.orderId = it->second.id;

        std::memset(body.symbol,0,sizeof(body.symbol));

        std::memcpy(body.symbol,it->second.symbol.c_str(),it->second.symbol.size());

        body.newPrice = RandomPrice();

        body.newQuantity = RandomQty();

        body.newSide = RandomSide();

        auto packet = Serializer::Serialize(PacketTpe::ModifyOrder,body);

        if(client.Send(packet))
        {
            it->second.price = body.newPrice;
            it->second.qty   = body.newQuantity;
            it->second.side  = body.newSide;
        }
    }

    void SendCancel()
    {
        if(activeOrders.empty()) return;

        auto it = RandomExisting();

        WireCancel body{};

        body.orderId = it->second.id;

        std::memset(body.symbol,0,sizeof(body.symbol));

        std::memcpy(body.symbol,it->second.symbol.c_str(),it->second.symbol.size());

        auto packet = Serializer::Serialize(PacketTpe::CancelOrder, body);

        client.Send(packet);

        activeOrders.erase(it);
    }

};

void ClientWorker(
    int id,
    const std::string& ip,
    uint16_t port,
    uint64_t packets
)
{
    TraderClient trader(
        id,
        ip,
        port,
        packets
    );

    trader.Run();
}

int main()
{
    constexpr int CLIENT_COUNT = 20;
    constexpr uint64_t PACKETS_PER_CLIENT = 100000;

    const std::string SERVER_IP = "127.0.0.1";
    const uint16_t SERVER_PORT = 8080;

    std::vector<std::thread> clients;

    auto start = std::chrono::high_resolution_clock::now();
/*
    std::cout << "Starting Stress Test\n";
    std::cout << "Clients            : " << CLIENT_COUNT << '\n';
    std::cout << "Packets / Client   : " << PACKETS_PER_CLIENT << '\n';
    std::cout << "Total Packets      : " << CLIENT_COUNT * PACKETS_PER_CLIENT << "\n";
*/

    for(int i = 0; i < CLIENT_COUNT; ++i)
    {
        clients.emplace_back(ClientWorker,i + 1,SERVER_IP,SERVER_PORT,PACKETS_PER_CLIENT);

        // Small stagger so all clients don't connect at the exact same instant.
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    for(auto &t : clients)
    {
        if(t.joinable()){
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "\n=========================================\n";
    std::cout << "Stress Test Finished\n";
    std::cout << "Elapsed Time : " << elapsed.count() << " ms\n";
    std::cout << "Total Clients: " << CLIENT_COUNT << '\n';
    std::cout << "Packets Sent : "
              << CLIENT_COUNT * PACKETS_PER_CLIENT
              << '\n';
    std::cout << "=========================================\n";

    return 0;
}