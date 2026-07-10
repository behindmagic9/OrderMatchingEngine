#include "../MatchingEngine/MatchingEngine.hpp"

bool MatchingEngine::DuplicateOrder(int orderId) {
    return orderIds.find(orderId) != orderIds.end();
}

void MatchingEngine::PrintOrderHistory() {
    std::cout << "\tID\t" << "Price\t" << "Original\t" << "Remain Quantity\t" << " Exec Quantity\t\t" << "Old Status\t" << "New Status\t" << std::endl;
    for (auto it = orderHistory.begin(); it != orderHistory.end(); it++) {
        int orderid = it->first;
        for (const auto& second : it->second) {
            std::cout << "\t" << orderid << "\t" << second.price << "\t" << second.originalquantity << "\t\t" << second.remquantity << "\t\t\t" << second.execquantity << "\t\t" << StatusToString(second.oldStatus) << "\t\t" << StatusToString(second.newStatus) << "\t" << std::endl;
        }
    }
}

void MatchingEngine::Consumer() {
    while (!qe.empty()) {
        auto temp = qe.front();
        qe.pop();

        switch (temp.type)
        {
            case CommandType::New:
            {
                Order order = temp.neworder.order;
                if (!Validator(order))
                {
                    std::cout << " validation of order id : " << order.orderId << " : failed with status : Rejected " << std::endl;
                    RecordOrderEvent(order, Status::REJECTED, 0);
                    break;
                }
                if (DuplicateOrder(order.orderId) && !temp.neworder.fromReplace) {
                    RecordOrderEvent(order, Status::REJECTED, 0);
                    break;
                }
                orderIds.insert(temp.neworder.order.orderId);
                ProcessOrder(order);
                break;
            }
            case CommandType::Cancel:
            {
                CancelOrder(temp.cancelOrder.orderId);
                break;
            }
            case CommandType::Modify:
            {
                ModifyOrder(temp.modifyOrder.orderId, temp.modifyOrder.newprice, temp.modifyOrder.newquantity, temp.modifyOrder.newside);
                break;
            }
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
    auto& SELL = this->orderBook.GetSellBook();
    MatchOrder(order, SELL,
        [](int64_t buyPrice, int64_t sellPrice)
        {
            return buyPrice >= sellPrice;
        }
    );
}

void MatchingEngine::ProcessSELL(Order& order)
{
    auto& BUY = this->orderBook.GetBuyBook();
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
                orderBook.RemovePointer(ord.orderId);
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
            ++it;;
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
            orderBook.AddToOrderBook(order);
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
            this->orderBook.AddToOrderBook(order);
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
            origquantity,
            execquantity,
            order.quantity,
        }
    );
    order.status = newStatus;
}

void MatchingEngine::Submit(const Command& cmd) {
    this->qe.push(cmd);
}

void MatchingEngine::ModifyOrder(int orderId, int64_t newprice, int newquantity, char newside)
{
    auto it = orderBook.OrderPointersStore.find(orderId);
    if (it == orderBook.OrderPointersStore.end()) {
        return;// so such order
    }
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
    CancelOrder(orderId);
    oldOrder.status = Status::NEW;
    Submit(Command::New(oldOrder, true));
}

void MatchingEngine::CancelOrder(int orderId) {
    auto cancelled = orderBook.CancelOrder(orderId);
    if (!cancelled.has_value()) {
        std::cout << "could not foudn respective Order id : " << orderId << std::endl;
        return;
    }
    RecordOrderEvent(*(cancelled), Status::CANCELLED, 0);
}