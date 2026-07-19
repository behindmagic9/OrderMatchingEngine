#pragma once

#include<string>
#include<optional>
#include<unordered_map>
#include<vector>
#include<atomic>

class SymbolRegistry{
    public:
        SymbolRegistry() {}
        std::optional<uint64_t> GetSymbolId(std::string_view symbol)const;
        std::optional<std::string_view> GetSymbolName(int id) const;
        size_t Size() const;
        
        private :
        std::unordered_map<std::string, uint64_t> mappingSymbol;
        std::atomic<int> nextId {100};
        std::vector<std::string> idtoSymbol;
        int RegisterSymbol(std::string symbol);
        friend class TradingServer;
};

class TradingServer{
    public:
        TradingServer(SymbolRegistry& registry) : _registry(registry){}
        void AddInstrument(std::string symbol){
            _registry.RegisterSymbol(std::move(symbol));
        }
    private:
        SymbolRegistry& _registry;
};
