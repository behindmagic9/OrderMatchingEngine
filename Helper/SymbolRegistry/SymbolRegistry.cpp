#include "../SymbolRegistry/SymbolRegistry.hpp"

std::optional<uint8_t> SymbolRegistry::GetSymbolId(const std::string_view& symbol) const{
    auto it = mappingSymbol.find(std::string(symbol));
    if(it == mappingSymbol.end()){
        return std::nullopt;
    }
    return it->second;
}

uint8_t SymbolRegistry::RegisterSymbol(const std::string& symbol){
    auto it = mappingSymbol.find(symbol);
    if(it != mappingSymbol.end()){
        return it->second;
    }

    int id = nextId.fetch_add(1);
    mappingSymbol.emplace(symbol, id);
    idtoSymbol.push_back(symbol);
    return id;
}

size_t SymbolRegistry::Size() const{
    return mappingSymbol.size();
}

std::optional<std::string_view> SymbolRegistry::GetSymbolName(const uint8_t& id)const {
    return idtoSymbol[id];
}