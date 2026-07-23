#include "Dispactcher/Dispatcher.hpp"
#include "Command.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono;

constexpr uint64_t ORDER_COUNT = 2'000'000;

int main()
{
    Dispatcher dispatcher;
    dispatcher.Start();

    auto start = high_resolution_clock::now();

    for (uint64_t i = 1; i <= ORDER_COUNT; i++)
    {
        OrderData order{};

        order.orderId = i;
        order.symbol = 1;
        order.side = (i & 1) ? 'B' : 'S';
        order.price = 100 + (i % 20);
        order.quantity = 10;
        order.otype = OrderType::Limit;
        order.otf = OrderTimeinFrame::GTC;

        auto shard = dispatcher.hashString(order.symbol);

        dispatcher.engineArray[shard].Submit(Command::New(std::move(order)));
    }

    while (true)
    {
        auto processed = dispatcher.ProcessedOrders();
        if (processed == ORDER_COUNT) break;
        static uint64_t counter = 0;

        if (++counter % 100000 == 0){
            std::this_thread::yield();
        }
        std::cout << processed << '\n';
    }

    auto end = high_resolution_clock::now();

    dispatcher.Close();

    double seconds = duration<double>(end - start).count();

    std::cout << "\n========== ENGINE BENCHMARK ==========\n";
    std::cout << "Orders        : " << ORDER_COUNT << '\n';
    std::cout << "Elapsed       : " << seconds << " sec\n";
    std::cout << "Throughput    : " << static_cast<double>(ORDER_COUNT) / seconds << " orders/sec\n";
}   