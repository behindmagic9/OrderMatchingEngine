#pragma once

#include<string>
#include<optional>
#include<unordered_map>
#include<vector>
#include<atomic>

class SymbolRegistry{
    public:
        SymbolRegistry() {}
        std::optional<uint8_t> GetSymbolId(const std::string_view& symbol)const;
        std::optional<std::string_view> GetSymbolName(const uint8_t& id) const;
        size_t Size() const;
        
        private :
        std::unordered_map<std::string, uint8_t> mappingSymbol;
        std::atomic<int> nextId {100};
        std::vector<std::string> idtoSymbol;
        uint8_t RegisterSymbol(const std::string& symbol);
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
