#include<thread>
#include<atomic>
#include "Dispactcher/Dispatcher.hpp"

void Producer(Dispatcher& dispatcher, std::atomic<int>& nextid) {
    dispatcher.submit(Command::New(Order{ nextid.fetch_add(1), 'B', 105, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    dispatcher.submit(Command::New(Order{ nextid.fetch_add(1), 'S', 103, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    dispatcher.submit(Command::New(Order{ nextid.fetch_add(1), 'B', 105, 100,"BTC" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    dispatcher.submit(Command::New(Order{ nextid.fetch_add(1), 'S', 103, 40, "BTC" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    dispatcher.submit(Command::New(Order{ nextid.fetch_add(1), 'B', 100, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    dispatcher.submit(Command::New(Order{ nextid.fetch_add(1), 'S', 101, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    dispatcher.submit(Command::New(Order{ nextid.fetch_add(1), 'B', 105, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    dispatcher.submit(Command::New(Order{ nextid.fetch_add(1), 'S', 100, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    dispatcher.submit(Command::Modify(5, "APPL", -1, -1, 'S'));
}

int main()
{
    Dispatcher d;
    d.Start();

    std::atomic<int> nextid{ 1 };

    std::thread t1(Producer, std::ref(d), std::ref(nextid));
    std::thread t2(Producer, std::ref(d), std::ref(nextid));
    std::thread t3(Producer, std::ref(d), std::ref(nextid));

    t1.join();
    t2.join();
    t3.join();

    d.Close();

    d.printTrades();

    d.PrintAllOrderBooks();
    d.PrintOrderHistory();
    return 0;
}