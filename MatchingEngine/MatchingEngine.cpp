#include "../MatchingEngine/MatchingEngine.hpp"

bool MatchingEngine::DuplicateOrder(int orderId) {
    return orderIds.find(orderId) != orderIds.end();
}

void MatchingEngine::PrintOrderHistory() {
    std::cout << "\tID\t" << "Symbol\t" <<"Price\t" << "Original\t" << "Remain Quantity\t" << " Exec Quantity\t\t" << "Old Status\t" << "New Status\t" << std::endl;
    for (auto it = orderHistory.begin(); it != orderHistory.end(); it++) {
        int orderid = it->first;
        for (const auto& second : it->second) {
            std::cout << "\t" << orderid << "\t" <<  second.symbol <<"\t"<< second.price << "\t" << second.originalquantity << "\t\t" << second.remquantity << "\t\t\t" << second.execquantity << "\t\t" << StatusToString(second.oldStatus) << "\t\t" << StatusToString(second.newStatus) << "\t" << std::endl;
        }
    }
}

void MatchingEngine::Consumer() {
    while (true) {
        Command temp;
        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [&]{
                return closed || !qe.empty();
            });
            if(closed  && qe.empty()){
                break;
            }
            temp = std::move(qe.front());
            qe.pop();
        }

        // get_if is used to safely inspect inside variant without thirowing exception and retriee values
        if(auto* neworder = std::get_if<NewOrder>(&temp.data)){
            //handle new order
            if (!Validator(neworder->order))
            {
                std::cout << " validation of order id : " << neworder->order.orderId << " : failed with status : Rejected " << std::endl;
                RecordOrderEvent(neworder->order, Status::REJECTED, 0);
                continue;
            }
            if (DuplicateOrder(neworder->order.orderId) && !neworder->fromReplace) {
                RecordOrderEvent(neworder->order, Status::REJECTED, 0);
                continue;
            }
            orderIds.insert(neworder->order.orderId);
            symbols_set.insert(neworder->order.symbol);
            ProcessOrder(neworder->order);
        }
        else if(auto* modifyOrder = std::get_if<Modify_Order>(&temp.data)){
            ModifyOrder(modifyOrder->orderId, modifyOrder->symbol, modifyOrder->newprice, modifyOrder->newquantity, modifyOrder->newside);
        }else if(auto* cancelOrder = std::get_if<Cancel_Order>(&temp.data)){
            CancelOrder(cancelOrder->orderId, cancelOrder->symbol);
        }
    }
}

void MatchingEngine::ProcessOrder(Order& order) {
    if (order.side == 'B')
    {
        ProcessBUY(order);
    }
    else {
        ProcessSELL(order);
    }
}

void MatchingEngine::ProcessBUY(Order& order)
{
    auto& SELL = GetOrderBook(order.symbol).GetSellBook();
    MatchOrder(order, SELL,
        [](int64_t buyPrice, int64_t sellPrice)
        {
            return buyPrice >= sellPrice;
        }
    );
}

void MatchingEngine::ProcessSELL(Order& order)
{
    auto& BUY = GetOrderBook(order.symbol).GetBuyBook();
    MatchOrder(order, BUY,
        [](int64_t sellPrice, int64_t buyPrice)
        {
            return buyPrice >= sellPrice;
        }
    );
}

template <typename OppositeBook, typename Compare>
bool MatchingEngine::CanFullFillOrder(const Order& order, OppositeBook& oppositeBook, Compare comp)
{
    int remaining = order.quantity;
    for (auto it = oppositeBook.begin(); it != oppositeBook.end(); it++)
    {
        if (order.otype == OrderType::Limit && !comp(order.price, it->first))
        {
            break;
        }
        for (auto& ord : it->second)
        {
            remaining -= ord.quantity;
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
    if (order.otf == OrderTimeinFrame::FOK)
    {
        if (!CanFullFillOrder(order, oppositeBook, comp))
        {
            RecordOrderEvent(order, Status::CANCELLED);
            order.quantity = 0;
            return;
        }
    }
    int originalQuantity = order.quantity;
    auto it = oppositeBook.begin();
    while (order.quantity > 0 && it != oppositeBook.end())
    {
        if (order.otype == OrderType::Limit && !comp(order.price, it->first))
        {
            break;
        }
        // rest market order type will skip above condition and jump to this and execute direclty wiht no price thing
        auto& q = it->second;
        while (order.quantity > 0 && !q.empty())
        {
            auto& ord = q.front();
            int quant = std::min(ord.quantity, order.quantity);

            // trade is heppenig so will record trade obejct here
            int64_t tradePrice = ord.price;
            RecordTrade(order, ord, quant, tradePrice);

            order.quantity -= quant;
            ord.quantity -= quant;
            int exec = quant;
            if (ord.quantity == 0) {
                RecordOrderEvent(ord, Status::FILLED, exec);
                GetOrderBook(ord.symbol).RemovePointer(ord.orderId);
                q.pop_front();
                break;
            }
            else {
                RecordOrderEvent(ord, Status::PARTIAL_FILLED, quant);
            }
        }
        if (order.quantity == 0) {
            break;
        }
        if (q.empty()) {
            it = oppositeBook.erase(it);
        }
        else {
            ++it;
        }
    }

    if (order.quantity == 0) {
        RecordOrderEvent(order, Status::FILLED, originalQuantity);
        std::cout << "order fullfilled " << std::endl;
    }
    else if (order.quantity < originalQuantity)
    {
        if (order.otype == OrderType::Market)
        {
            RecordOrderEvent(order, Status::PARTIAL_FILLED_Cancel, originalQuantity - order.quantity, originalQuantity);
            order.quantity = 0;
        }
        else if (order.otf == OrderTimeinFrame::GTC)
        {
            RecordOrderEvent(order, Status::PARTIAL_FILLED, originalQuantity - order.quantity, originalQuantity);
            // add to otder book
            GetOrderBook(order.symbol).AddToOrderBook(order);
        }
        else if (order.otf == OrderTimeinFrame::IOC)
        {
            // cancle order now
            std::cout << "filled is : " << originalQuantity - order.quantity << ": remainging are cancelled : " << order.quantity << std::endl;
            RecordOrderEvent(order, Status::PARTIAL_FILLED_Cancel, originalQuantity - order.quantity, originalQuantity);
            order.quantity = 0; // reseting the quantity
        }
    }
    else
    {
        if (order.otype == OrderType::Market)
        {
            RecordOrderEvent(order, Status::CANCELLED, 0);
            order.quantity = 0;
        }
        else if (order.otf == OrderTimeinFrame::GTC)
        {
            RecordOrderEvent(order, Status::OPEN);
            // add to otder book
            GetOrderBook(order.symbol).AddToOrderBook(order);
        }
        else if (order.otf == OrderTimeinFrame::IOC)
        {
            // cancle order now
            RecordOrderEvent(order, Status::CANCELLED, 0);
            std::cout << "nothing matched so cancelling it" << std::endl;
            order.quantity = 0; // reseting the quantity
        }
    }
}

void MatchingEngine::RecordOrderEvent(Order& order, Status newStatus, int execquantity, int origquantity) {

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

void MatchingEngine::Submit(const Command& cmd) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        if(closed) return;
        qe.push(std::move(cmd));
    }
    cv.notify_one();
}

void MatchingEngine::ModifyOrder(int orderId,std::string symbol, int64_t newprice, int newquantity, char newside)
{
    // see if order already exist in there or not
    auto it = GetOrderBook(symbol).OrderPointersStore.find(orderId);
    if (it == GetOrderBook(symbol).OrderPointersStore.end()) {
        return;// so such order
    }

    // if exist ,then using the pointer from orderpointerstore get that order
    Order oldOrder = *(it->second.iterator);
    bool priceChanged = (newprice != -1 && newprice != oldOrder.price);
    bool quantityIncreased = (newquantity != -1 && newquantity > oldOrder.quantity);
    bool sideChanged = (newside != '\0' && newside != oldOrder.side);

    if (!priceChanged && !quantityIncreased && !sideChanged) {
        if (newquantity != -1) {
            if (newquantity <= 0) {
                std::cout << "invalid quantity" << std::endl;
                return;
            }
            it->second.iterator->quantity = newquantity;
        }
        return;
    }
    if (!(newprice == -1)) {
        oldOrder.price = newprice;
    }
    if (!(newquantity == -1)) {
        oldOrder.quantity = newquantity;
    }
    if (!(newside == '\0')) {
        oldOrder.side = newside;
    }

    if (!Validator(oldOrder)) {
        std::cout << " invalid mofications : rejected by validator" << std::endl;
        return;
    }

    //Nee cancel order ad  repalce
    CancelOrder(orderId, oldOrder.symbol);
    oldOrder.status = Status::NEW;
    Submit(Command::New(oldOrder, true));
}

void MatchingEngine::CancelOrder(int orderId, std::string symbol) {
    auto cancelled = GetOrderBook(symbol).CancelOrder(orderId);
    if (!cancelled.has_value()) {
        std::cout << "could not foudn respective Order id : " << orderId << std::endl;
        return;
    }
    RecordOrderEvent(*(cancelled), Status::CANCELLED, 0);
}

void MatchingEngine::PrintAllOrderBooks(){
    for(auto symbol : symbols_set){
        std::cout << "\n\n==== Start ====" << std::endl;
        std::cout << "Symbol : " << symbol << std::endl;
        GetOrderBook(symbol).PrintOrderBook();
        std::cout << "====== End ========\n" << std::endl;
    }
}

void MatchingEngine::CloseQueue(){
    {
        std::lock_guard<std::mutex> lock(mtx);
        closed = true;
    }
    cv.notify_all();
}
