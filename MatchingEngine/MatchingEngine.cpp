#include "../MatchingEngine/MatchingEngine.hpp"

bool MatchingEngine::DuplicateOrder(uint64_t orderId) {
    return orderIds.find(orderId) != orderIds.end();
}

/*
void MatchingEngine::PrintOrderHistory() {
    std::cout << "\tID\t" << "Symbol\t" << "Price\t" << "Original\t" << "Remain Quantity\t" << " Exec Quantity\t\t" << "Old Status\t" << "New Status\t" << std::endl;
    for (auto it = orderHistory.begin(); it != orderHistory.end(); it++) {
        int orderid = it->first;
        for (const auto& second : it->second) {
            std::cout << "\t" << orderid << "\t" << second.symbol << "\t" << second.price << "\t" << second.originalquantity << "\t\t" << second.remquantity << "\t\t\t" << second.execquantity << "\t\t" << StatusToString(second.oldStatus) << "\t\t" << StatusToString(second.newStatus) << "\t" << std::endl;
        }
    }
}
*/

void MatchingEngine::Consumer() {
    while (true) {

        if (stop.load(std::memory_order_acquire) && qe.empty()) break;
        auto temp = qe.pop();

        if(!temp){
            std::this_thread::yield();
            continue;
        }

        // now changing everyone from . to -> cause the sd::optional in pop return the pointer which need this ->
        // get_if is used to safely inspect inside variant without thirowing exception and retriee values
        if(auto* neworder = std::get_if<NewOrder>(&temp->data)){
            //handle new order
            if (!Validator(neworder->order))
            {
                //RecordOrderEvent(neworder->order.data, Status::REJECTED, 0);
                continue;
            }
            if (DuplicateOrder(neworder->order.orderId) && !neworder->fromReplace) {
                //RecordOrderEvent(neworder->order.data, Status::REJECTED, 0);
                continue;
            }
            orderIds.insert(neworder->order.orderId);
            //symbols_set.insert(neworder->order.data.symbol);
            Order order{};
            order.data = std::move(neworder->order);
            ProcessOrder(order);
        }
        else if(auto* modifyOrder = std::get_if<Modify_Order>(&temp->data)){
            ModifyOrder(modifyOrder->orderId, modifyOrder->symbol, modifyOrder->newprice, modifyOrder->newquantity, modifyOrder->newside);
        }else if(auto* cancelOrder = std::get_if<Cancel_Order>(&temp->data)){
            CancelOrder(cancelOrder->orderId, cancelOrder->symbol);
        }
    }
}

void MatchingEngine::ProcessOrder(Order& order) {
    if (order.data.side == 'B')
    {
        ProcessBUY(order);
    }
    else {
        ProcessSELL(order);
    }
}

void MatchingEngine::ProcessBUY(Order& order)
{
    auto& SELL = GetOrderBook(order.data.symbol).GetSellBook();
    MatchOrder(order, SELL,
        [](uint32_t buyPrice, uint32_t sellPrice)
        {
            return buyPrice >= sellPrice;
        }
    );
}

void MatchingEngine::ProcessSELL(Order& order)
{
    auto& BUY = GetOrderBook(order.data.symbol).GetBuyBook();
    MatchOrder(order, BUY,
        [](uint32_t sellPrice, uint32_t buyPrice)
        {
            return buyPrice >= sellPrice;
        }
    );
}

template <typename OppositeBook, typename Compare>
bool MatchingEngine::CanFullFillOrder(const Order& order, OppositeBook& oppositeBook, Compare comp)
{
    uint32_t remaining = order.data.quantity;
    for (auto it = oppositeBook.begin(); it != oppositeBook.end(); it++)
    {
        if (order.data.otype == OrderType::Limit && !comp(order.data.price, it->first))
        {
            break;
        }
        for (auto& ord : it->second)
        {
            remaining -= ord.data.quantity;
            if (remaining <= 0)
            {
                return true;
            }
        }
    }
    return false;
}

// here the Opposite BOok is deined by type whihc defined on calling the MAtchOrder fucntion
// the compare here work as the Lambda Obejct type whcih deduce the comp with lambda() operator , whihc is what we define in the Calling when ProcessBUy and ProcessSEll of like
/*
class Lambda
{
public: bool Opertor() (int buy price , int sellPrice ){
return ..........
}
// as this condition is differnt in both buy and sell so make it lambda so that can independtly declare and check that and perform correctly
}
*/
template <typename OppositeBook, typename Compare>
void MatchingEngine::MatchOrder(Order& order, OppositeBook& oppositeBook, Compare comp)
{
    if (order.data.otf == OrderTimeinFrame::FOK)
    {
        if (!CanFullFillOrder(order, oppositeBook, comp))
        {
            //RecordOrderEvent(order, Status::CANCELLED);
            order.data.quantity = 0;
            return;
        }
    }
    uint32_t originalQuantity = order.data.quantity;
    auto it = oppositeBook.begin();
    while (order.data.quantity > 0 && it != oppositeBook.end())
    {
        if (order.data.otype == OrderType::Limit && !comp(order.data.price, it->first))
        {
            break;
        }
        // rest market order type will skip above condition and jump to this and execute direclty wiht no price thing
        auto& q = it->second;
        while (order.data.quantity > 0 && !q.empty())
        {
            auto current = q.begin();
            auto& ord = *current;
            uint32_t quant = std::min(ord.data.quantity, order.data.quantity);

            // trade is heppenig so will record trade obejct here
            uint32_t tradePrice = ord.data.price;
            //RecordTrade(order, ord, quant, tradePrice);

            order.data.quantity -= quant;
            ord.data.quantity -= quant;
            uint32_t exec = quant;
            if (ord.data.quantity == 0) {
                Order* ptr = &(ord);
                //RecordOrderEvent(ord, Status::FILLED, exec);
                GetOrderBook(ptr->data.symbol).RemovePointer(ptr->data.orderId);
                q.erase(current);
                GetOrderBook(ptr->data.symbol).ReleaseOrder(ptr);
                continue;
            }
            else {
                //RecordOrderEvent(ord, Status::PARTIAL_FILLED, quant);
            }
        }
        if (q.empty()) {
            it = oppositeBook.erase(it);
        }
        else {
            ++it;
        }
    }

    if (order.data.quantity == 0) {
        //RecordOrderEvent(order, Status::FILLED, originalQuantity);
    }
    else if (order.data.quantity < originalQuantity)
    {
        if (order.data.otype == OrderType::Market)
        {
            //RecordOrderEvent(order, Status::PARTIAL_FILLED_Cancel, originalQuantity - order.quantity, originalQuantity);
            order.data.quantity = 0;
        }
        else if (order.data.otf == OrderTimeinFrame::GTC)
        {
           // RecordOrderEvent(order, Status::PARTIAL_FILLED, originalQuantity - order.quantity, originalQuantity);
            // add to otder book
            GetOrderBook(order.data.symbol).AddToOrderBook(std::move(order));
        }
        else if (order.data.otf == OrderTimeinFrame::IOC)
        {
            // cancle order now
            //std::cout << "filled is : " << originalQuantity - order.quantity << ": remainging are cancelled : " << order.quantity << std::endl;
            //RecordOrderEvent(order, Status::PARTIAL_FILLED_Cancel, originalQuantity - order.quantity, originalQuantity);
            order.data.quantity = 0; // reseting the quantity
        }
    }
    else
    {
        if (order.data.otype == OrderType::Market)
        {
            //RecordOrderEvent(order, Status::CANCELLED, 0);
            order.data.quantity = 0;
        }
        else if (order.data.otf == OrderTimeinFrame::GTC)
        {
            //RecordOrderEvent(order, Status::OPEN);
            // add to otder book
            GetOrderBook(order.data.symbol).AddToOrderBook(std::move(order));
        }
        else if (order.data.otf == OrderTimeinFrame::IOC)
        {
            // cancle order now
            //RecordOrderEvent(order, Status::CANCELLED, 0);
            order.data.quantity = 0; // reseting the quantity
        }
    }
}

/*
void MatchingEngine::RecordOrderEvent(Order& order, Status newStatus, uint64_t execquantity, uint64_t origquantity) {

    if (origquantity == -1) {
        origquantity = order.quantity + execquantity;
    }
    orderHistory[order.orderId].push_back(
        {
            order.status,
            newStatus,
            order.price,
            order.symbol,
            origquantity,
            execquantity,
            order.quantity,
        }
    );
    order.status = newStatus;
}
*/

void MatchingEngine::Submit(Command&& cmd) {
    qe.push(std::move(cmd));
}

void MatchingEngine::ModifyOrder(uint64_t orderId,uint8_t symbol, uint32_t newprice, uint32_t newquantity, char newside)
{
    // see if order already exist in there or not
    auto it = GetOrderBook(symbol).OrderPointersStore.find(orderId);
    if (it == GetOrderBook(symbol).OrderPointersStore.end()) {
        return;// so such order
    }

    // if exist ,then using the pointer from orderpointerstore get that order
    auto oldOrder = (it->second.iterator);
    bool priceChanged = (newprice != -1 && newprice != oldOrder->data.price);
    bool quantityIncreased = (newquantity != -1 && newquantity > oldOrder->data.quantity);
    bool sideChanged = (newside != '\0' && newside != oldOrder->data.side);

    if (!priceChanged && !quantityIncreased && !sideChanged) {
        if (newquantity != -1) {
            if (newquantity <= 0) {
                return;
            }
            it->second.iterator->data.quantity = newquantity;
        }
        return;
    }
    OrderData replacement = oldOrder->data;
    if (!(newprice == -1)) {
        replacement.price = newprice;
    }
    if (!(newquantity == -1)) {
        replacement.quantity = newquantity;
    }
    if (!(newside == '\0')) {
        replacement.side = newside;
    }

    if (!Validator(replacement)) {
        return;
    }

    //Nee cancel order ad  repalce
    CancelOrder(orderId, replacement.symbol);
    replacement.status = Status::NEW;
    Submit(Command::New(std::move(replacement), true));
}

void MatchingEngine::CancelOrder(uint64_t orderId, uint8_t symbol) {
    auto cancelled = GetOrderBook(symbol).CancelOrder(orderId);
    if (!cancelled.has_value()) {
        return;
    }
    //RecordOrderEvent(*(cancelled), Status::CANCELLED, 0);
}

/*
void MatchingEngine::PrintAllOrderBooks(){
    for(auto symbol : symbols_set){
        std::cout << "Symbol : " << symbol << std::endl;
        GetOrderBook(symbol).PrintOrderBook();
        std::cout << "====== End ========\n" << std::endl;
    }
}
*/


/*
void MatchingEngine::RecordTrade(Order& incoming, Order& recieving, uint64_t quantity, uint64_t price) {
    Trade t{
        incoming.side == 'B' ? incoming.orderId : recieving.orderId,
        incoming.side == 'S' ? incoming.orderId : recieving.orderId,
        quantity,
        price };

    trades.push_back(t);
}
*/

MatchingEngine::MatchingEngine(){
    orderIds.reserve(4096);
}

void MatchingEngine::Stop(){
    stop.store(true, std::memory_order_release);
}