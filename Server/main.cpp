#include "../Network/TCPServer.hpp"
#include "../Dispactcher/Dispatcher.hpp"
#include "../Network/Encode/Deserializer.hpp"
#include "../Helper/SymbolRegistry/SymbolRegistry.hpp"

int main(){
    Dispatcher _dispatcher;
    SymbolRegistry registry;
    TradingServer adminRegistry(registry);
    adminRegistry.AddInstrument("BTC");
    adminRegistry.AddInstrument("ETH");
    adminRegistry.AddInstrument("AAPL");
    adminRegistry.AddInstrument("MSFT");
    adminRegistry.AddInstrument("AMD");
    adminRegistry.AddInstrument("GOOG");
    adminRegistry.AddInstrument("TSLA");
    adminRegistry.AddInstrument("NVDA");

    Deserializer _deserializer(registry);
    _dispatcher.Start();
    TCPServer server(8080 , _dispatcher ,_deserializer);

    server.Start();
    
    std::cout << "Press Enter to stop.. \n";
    std::cin.get();

    _dispatcher.Close();
    /*
    _dispatcher.PrintOrderHistory();
    _dispatcher.printTrades();
    */

}

/*
g++ -g -std=c++17 -pthread Server/main.cpp MatchingEngine/MatchingEngine.cpp OrderBook/OrderBook.cpp Dispactcher/Dispatcher.cpp Network/TCPSever.cpp Helper/SymbolRegistry/SymbolRegistry.cpp -I. -o server

*/