#include <algorithm>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <assert.h>

#include "order.hpp"
#include "order_book.hpp"

template void OrderBook::Handle<Side::BUY>(std::shared_ptr<Order> order);
template void OrderBook::Handle<Side::SELL>(std::shared_ptr<Order> order);
template void OrderBook::Add<Side::SELL>(std::shared_ptr<Order> order);
template void OrderBook::Add<Side::BUY>(std::shared_ptr<Order> order);
template bool OrderBook::Execute<Side::SELL>(std::shared_ptr<Order> order);
template bool OrderBook::Execute<Side::BUY>(std::shared_ptr<Order> order);
template void OrderBook::Cancel<Side::SELL>(std::shared_ptr<Order> order);
template void OrderBook::Cancel<Side::BUY>(std::shared_ptr<Order> order);

template <Side side>
void OrderBook::Handle(std::shared_ptr<Order> order)
{
    assert(order->GetSide() == side);
    assert(order->GetActivated() == false);

    {
        SyncInfo() << "[HANDLE WAITING] Order: " << order->GetOrderId() << ", for BOTH LOCKS\n";
        std::unique_lock<std::mutex> l(order_book_lock);
        SyncInfo() << "[HANDLE] Order: " << order->GetOrderId() << ",  Acquired BOTH LOCKS\n";
        // Get timestamp for order
        order->SetTimestamp(getCurrentTimestamp());

        // Insert dummy node into order
        Add<side>(order);

        SyncInfo() << "[HANDLE RELEASE] Order: " << order->GetOrderId() << ",  BOTH LOCKS\n";
    }

    // Execute
    bool filled = Execute<side>(order);
    SyncInfo() << "[HANDLE WAITING] Order: " << order->GetOrderId() << ",  for" << (side == Side::BUY ? "BUY" : "SELL") << " lock!"
               << std::endl;
    std::unique_lock<std::mutex> lo;
    if constexpr (side == Side::BUY)
        lo = std::unique_lock<std::mutex>(bids_lock);
    else
        lo = std::unique_lock<std::mutex>(asks_lock);
    SyncInfo() << "[HANDLE] Order: " << order->GetOrderId() << ",  Acquire " << (side == Side::BUY ? "BUY" : "SELL") << " lock!"
               << std::endl;
    if (!filled)
        Output::OrderAdded(
            order->GetOrderId(),
            order->GetInstrumentId().c_str(),
            order->GetPrice(),
            order->GetCount(),
            side == Side::SELL,
            getCurrentTimestamp());

    // Add
    order->Activate();
    SyncInfo() << "[***HANDLE RELEASE] Order: " << order->GetOrderId() << ", " << (side == Side::BUY ? "BUY" : "SELL") << " lock!"
               << std::endl;
}

template <Side side>
void OrderBook::Add(std::shared_ptr<Order> order)
{
    assert(order->GetSide() == side);
    SyncInfo() << "[ADD WAITING] Order: " << order->GetOrderId() << ",  for" << (side == Side::BUY ? "BUY" : "SELL") << " lock!"
               << std::endl;
    std::unique_lock<std::mutex> l;
    if constexpr (side == Side::BUY)
        l = std::unique_lock<std::mutex>(bids_lock);
    else
        l = std::unique_lock<std::mutex>(asks_lock);
    SyncInfo() << "[ADD] Order: " << order->GetOrderId() << ", Acquire " << (side == Side::BUY ? "BUY" : "SELL") << " lock!" << std::endl;
    std::shared_ptr<Price> p = GetPrice<side>(order->GetPrice());
    p->push_back(order);
    SyncInfo() << "[ADD RELEASE] Order: " << order->GetOrderId() << ", " << (side == Side::BUY ? "BUY" : "SELL") << " lock!" << std::endl;
}

/**
 * @param order active buy order to execute.
 * @return if buy order has been fully completed.
 */
template <Side side>
bool OrderBook::Execute(std::shared_ptr<Order> order)
{
    assert(order->GetSide() == side);
    assert(order->GetActivated() == false);
    std::unique_lock<std::mutex> l(side == Side::BUY ? asks_lock : bids_lock);

    // Check if any sell orders
    if constexpr (side == Side::BUY)
    {
        if (asks.size() == 0)
            return false;
    }
    else
    {
        if (bids.size() == 0)
            return false;
    }
    // Check if lowest sell order can match the buy
    auto firstEl = ([&]() {
        if constexpr (side == Side::BUY) 
        return asks.begin();
        else 
        return bids.begin(); 
    })();
    auto lastEl = ([&]() {
        if constexpr (side == Side::BUY) 
        return asks.end();
        else 
        return bids.end(); 
    })();

    while (firstEl != lastEl)
    {
        price_t price = firstEl->first;
        std::shared_ptr<Price> priceQueue = firstEl->second;

        SyncInfo() << "[EXECUTE] Order: " << order->GetOrderId() << ". Order Price: " << order->GetPrice()
                   << ", Order count: " << order->GetCount() << ", oppPrice: " << price << ", priceQueueSize: " << priceQueue->size()
                   << std::endl;
        if constexpr (side == Side::BUY)
        {
            if (price > order->GetPrice())
                return order->GetCount() == 0;
        }
        else
        {
            if (price < order->GetPrice())
                return order->GetCount() == 0;
        }

        // Iteratively match with all orders in this price queue.
        while (order->GetCount() > 0 && priceQueue->size())
        {
            std::shared_ptr<Order> oppOrder = priceQueue->front();
            // Check if first order's timestamp comes before the current buy
            if (oppOrder->GetTimestamp() > order->GetTimestamp())
                break;

            // Check if sell order is activated
            while (!oppOrder->GetActivated())
            {
                SyncInfo() << "[EXECUTE] Order: " << order->GetOrderId() << " going to sleep" << std::endl;
                oppOrder->cv.wait(l);
            }

            if (oppOrder->GetCompleted())
                continue;

            // Check if dummy order has already been filled
            if (oppOrder->GetCount() > 0)
            {
                oppOrder->IncrementExecutionId();
                MatchOrders(order, oppOrder);
            }
            if (oppOrder->GetCount() == 0)
            {
                oppOrder->SetCompleted();
                priceQueue->pop_front();
            }
        }
        firstEl++;
    }
    SyncInfo() << "[EXECUTE RELEASE] Order: " << order->GetOrderId() << ",  " << (side == Side::BUY ? "SELL" : "BUY") << " lock!"
               << std::endl;
    return order->GetCount() == 0;
}

template <Side side>
void OrderBook::Cancel(std::shared_ptr<Order> order)
{
    assert(order->GetSide() == side);
    std::unique_lock<std::mutex> l(side == Side::BUY ? bids_lock : asks_lock);
    while (!order->GetActivated())
        order->cv.wait(l);

    std::shared_ptr<Price> priceQueue = GetPrice<side>(order->GetPrice());
    std::deque<std::shared_ptr<Order>>::iterator start;
    for (start = priceQueue->begin(); start != priceQueue->end(); start++)
        if ((*start)->GetOrderId() == order->GetOrderId())
            break;

    // No order found
    if (start == priceQueue->end())
    {
        Output::OrderDeleted(order->GetOrderId(), false, getCurrentTimestamp());
        return;
    }

    std::shared_ptr<Order> o = *start;
    if (o->GetCompleted())
    {
        Output::OrderDeleted(order->GetOrderId(), false, getCurrentTimestamp());
        return;
    }
    int cnt = o->GetCount();
    o->SetCompleted();
    priceQueue->erase(start);

    Output::OrderDeleted(order->GetOrderId(), cnt > 0, getCurrentTimestamp());
}

void OrderBook::MatchOrders(std::shared_ptr<Order> incoming, std::shared_ptr<Order> resting)
{
    unsigned int qty = std::min(incoming->GetCount(), resting->GetCount());
    incoming->Fill(qty);
    resting->Fill(qty);
    Output::OrderExecuted(
        resting->GetOrderId(), incoming->GetOrderId(), resting->GetExecutionId(), resting->GetPrice(), qty, getCurrentTimestamp());
}

template <typename T>
std::shared_ptr<Price> GetOrAssign(std::map<price_t, std::shared_ptr<Price>, T> & map, price_t price)
{
    if (!map.contains(price))
        map[price] = std::make_shared<Price>();
    return map[price];
}

template <>
std::shared_ptr<Price> OrderBook::GetPrice<Side::BUY>(price_t price)
{
    return GetOrAssign(bids, price);
}

template <>
std::shared_ptr<Price> OrderBook::GetPrice<Side::SELL>(price_t price)
{
    return GetOrAssign(asks, price);
}
