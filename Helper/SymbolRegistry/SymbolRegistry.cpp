#include "../SymbolRegistry/SymbolRegistry.hpp"

std::optional<uint64_t> SymbolRegistry::GetSymbolId(std::string_view symbol) const{
    auto it = mappingSymbol.find(std::string(symbol));
    if(it == mappingSymbol.end()){
        return std::nullopt;
    }
    return it->second;
}

int SymbolRegistry::RegisterSymbol(std::string symbol){
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

std::optional<std::string_view> SymbolRegistry::GetSymbolName(int id)const {
    return idtoSymbol[id];
}