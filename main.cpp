#include "MatchingEngine/MatchingEngine.hpp"

void Producer(MatchingEngine& engine) {
    int id = 1;
    engine.Submit(Command::New(Order{ id++, 'B', 105, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'S', 103, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'B', 105, 100,"BTC" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'S', 103, 40, "BTC" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'B', 100, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'S', 101, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'B', 105, 100,"APPL" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'S', 100, 100,"ETH" ,OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::Modify(5,"APPL", - 1, -1, 'S'));
}

int main()
{
    MatchingEngine engine;
    Producer(engine);
    engine.Consumer();
    PrintTrades();
    engine.PrintAllOrderBooks();
    engine.PrintOrderHistory();
    return 0;
}