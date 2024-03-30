#include <algorithm>
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
template std::shared_ptr<Price> OrderBook::GetPrice<Side::SELL>(price_t price);
template std::shared_ptr<Price> OrderBook::GetPrice<Side::BUY>(price_t price);

template <Side side>
void OrderBook::Handle(std::shared_ptr<Order> order)
{
    assert(side == Side::SELL || side == Side::BUY);
    assert(order->GetSide() == side);
    assert(order->GetActivated() == false);
    SyncInfo() << "[HANDLE WAITING] Order: " << order->GetOrderId() << ", for BOTH LOCKS\n";
    std::unique_lock<std::mutex> l(order_book_lock);
    SyncInfo() << "[HANDLE] Order: " << order->GetOrderId() << ",  Acquired BOTH LOCKS\n";
    // Get timestamp for order
    order->SetTimestamp(getCurrentTimestamp());

    // Insert dummy node into order
    if constexpr (side == Side::BUY)
        Add<Side::BUY>(order);
    else
        Add<Side::SELL>(order);

    SyncInfo() << "[HANDLE RELEASE] Order: " << order->GetOrderId() << ",  BOTH LOCKS\n";
    l.unlock();

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
    assert(side == Side::SELL || side == Side::BUY);
    assert(order->GetSide() == side);
    assert(order->GetActivated() == false);
    std::unique_lock<std::mutex> l;
    SyncInfo() << "[EXECUTE WAITING] Order: " << order->GetOrderId() << ",  for" << (side == Side::BUY ? "SELL" : "BUY") << " lock!"
               << std::endl;
    if constexpr (side == Side::BUY)
        l = std::unique_lock<std::mutex>(asks_lock);
    else
        l = std::unique_lock<std::mutex>(bids_lock);

    SyncInfo() << "[EXECUTE] Order: " << order->GetOrderId() << ",  Acquire " << (side == Side::BUY ? "SELL" : "BUY") << " lock!"
               << std::endl;
    // Check if any sell orders
    if constexpr (side == Side::BUY)
    {
        if (asks.Size() == 0)
            return false;
    }
    else
    {
        if (bids.Size() == 0)
            return false;
    }
    int tmp = 0;
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
        assert(firstEl->second.initialised);
        std::shared_ptr<Price> priceQueue = firstEl->second.Get();

        if (tmp < 5)
            SyncInfo() << "[EXECUTE] Order: " << order->GetOrderId() << ". Order Price: " << order->GetPrice()
                       << ", Order count: " << order->GetCount() << ", oppPrice: " << price << ", priceQueueSize: " << priceQueue->size()
                       << std::endl;
        tmp++;
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

            // Check if dummy order has already been filled
            if (oppOrder->GetCount() == 0)
            {
                priceQueue->pop_front();
                continue;
            }
            oppOrder->IncrementExecutionId();
            MatchOrders(order, oppOrder);
            if (oppOrder->GetCount() == 0)
                priceQueue->pop_front();
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
    std::unique_lock<std::mutex> l;
    SyncInfo() << "[CANCEL WAITING] Order: " << order->GetOrderId() << ",  for" << (side == Side::BUY ? "BUY" : "SELL") << " lock!"
               << std::endl;
    if constexpr (side == Side::BUY)
        l = std::unique_lock<std::mutex>(bids_lock);
    else
        l = std::unique_lock<std::mutex>(asks_lock);

    SyncInfo() << "[CANCEL] Order: " << order->GetOrderId() << ", Acquire " << (side == Side::BUY ? "BUY" : "SELL") << " lock!"
               << std::endl;
    std::shared_ptr<Price> priceLevel = GetPrice<side>(order->GetPrice());
    bool found = false;
    for (auto start = priceLevel->begin(); start != priceLevel->end(); start++)
    {
        std::shared_ptr<Order> p = *start;
        while (!p->GetActivated())
            p->cv.wait(l);

        if (order->GetOrderId() == p->GetOrderId() && p->GetCount() > 0)
        {
            found = true;
            priceLevel->erase(start);
        }
    }
    if (priceLevel->size() == 0)
    {
        // Remove from map of prices
        size_t num = ([&]() -> size_t {
                if constexpr (side == Side::BUY) 
                    return bids.Erase(order->GetPrice());
                else 
                    return asks.Erase(order->GetPrice()); 
            })();
        assert(num == 1);
    }
    Output::OrderDeleted(order->GetOrderId(), found, getCurrentTimestamp());
    SyncInfo() << "[CANCEL RELEASE] Order: " << order->GetOrderId() << ", " << (side == Side::BUY ? "BUY" : "SELL") << " lock!"
               << std::endl;
}

void OrderBook::MatchOrders(std::shared_ptr<Order> incoming, std::shared_ptr<Order> resting)
{
    unsigned int qty = std::min(incoming->GetCount(), resting->GetCount());
    if (resting->GetCount() == incoming->GetCount())
    {
        incoming->Fill();
        resting->Fill();
    }
    else if (resting->GetCount() > incoming->GetCount())
    {
        resting->Fill(incoming->GetCount());
        incoming->Fill();
    }
    else
    {
        incoming->Fill(resting->GetCount());
        resting->Fill();
    }
    Output::OrderExecuted(
        resting->GetOrderId(), incoming->GetOrderId(), resting->GetExecutionId(), resting->GetPrice(), qty, getCurrentTimestamp());
}

template <Side side>
std::shared_ptr<Price> OrderBook::GetPrice(price_t price)
{
    WrapperValue<std::shared_ptr<Price>> &w = ([&]() {
      if constexpr (side == Side::BUY)
        return std::ref(bids.Get(price));
      else
        return std::ref(asks.Get(price)); 
    })();

    std::unique_lock<std::mutex> l(w.lock);
    if (!w.initialised)
    {
        w.initialised = true;
        w.val = std::make_shared<Price>();
    }
    return w.val;
}