#include "MatchingEngine/MatchingEngine.hpp"
#include<thread>
#include<atomic>

void Producer(MatchingEngine& engine, std::atomic<int>& nextid) {
    engine.Submit(Command::New(Order{ nextid.fetch_add(1), 'B', 105, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ nextid.fetch_add(1), 'S', 103, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ nextid.fetch_add(1), 'B', 105, 100,"BTC" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ nextid.fetch_add(1), 'S', 103, 40, "BTC" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ nextid.fetch_add(1), 'B', 100, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ nextid.fetch_add(1), 'S', 101, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ nextid.fetch_add(1), 'B', 105, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ nextid.fetch_add(1), 'S', 100, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::Modify(5,"APPL", - 1, -1, 'S'));
}

int main()
{
    MatchingEngine engine;
    std::thread tconsumer(&MatchingEngine::Consumer, &engine);
    std::atomic<int> nextid{1};
    std::thread t1(Producer, std::ref(engine), std::ref(nextid));
    std::thread t2(Producer, std::ref(engine), std::ref(nextid));
    std::thread t3(Producer, std::ref(engine), std::ref(nextid));
    t1.join();
    t2.join();
    t3.join();
    engine.CloseQueue();
    // after only a cancel command in done 
    tconsumer.join();
    PrintTrades();
    engine.PrintAllOrderBooks();
    engine.PrintOrderHistory();
    return 0;
}