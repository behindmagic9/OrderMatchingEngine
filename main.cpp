#include "MatchingEngine/MatchingEngine.hpp"

void Producer(MatchingEngine& engine) {
    int id = 1;
    engine.Submit(Command::New(Order{ id++, 'B', 105, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'S', 103, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'B', 105, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'S', 103, 40, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'B', 100, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'S', 101, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'B', 105, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::New(Order{ id++, 'S', 100, 100, OrderType::Limit, OrderTimeinFrame::GTC, Status::NEW }));
    engine.Submit(Command::Modify(5, -1, -1, 'S'));
}

int main()
{
    MatchingEngine engine;
    Producer(engine);
    engine.Consumer();
    PrintTrades();
    engine.GetOrderBook().PrintOrderBook();
    engine.PrintOrderHistory();
    return 0;
}