#include <algorithm>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <assert.h>

#include "order.hpp"
#include "order_book.hpp"

void OrderBook::Handle(std::shared_ptr<Order> order)
{
    assert(order->GetActivated() == false);

    Prepare(order);

    Execute(order);
}

// Set arrival timestamp for order and add dummy node into book
void OrderBook::Prepare(std::shared_ptr<Order> order)
{
    std::unique_lock<std::mutex> l(order_book_lock);

    // Get timestamp for order
    order->SetTimestamp(getCurrentTimestamp());

    // Insert dummy node into order
    Add(order);
}


void OrderBook::Add(std::shared_ptr<Order> order)
{
    std::unique_lock<std::mutex> l(order->GetSide() == Side::BUY ? bids_lock : asks_lock);
    std::shared_ptr<Price> p = GetPrice(order->GetSide(), order->GetPrice());
    p->push_back(order);
}

void MatchOrders(std::shared_ptr<Order> incoming, std::shared_ptr<Order> resting)
{
    unsigned int qty = std::min(incoming->GetCount(), resting->GetCount());
    incoming->Fill(qty);
    resting->Fill(qty);
    Output::OrderExecuted(
        resting->GetOrderId(), incoming->GetOrderId(), resting->GetExecutionId(), resting->GetPrice(), qty, getCurrentTimestamp());
}

template <typename T>
bool CrossSpread(Book<T> & book, std::shared_ptr<Order> order, std::unique_lock<std::mutex> & l)
{
    if (book.size() == 0)
        return false;

    for (auto & [price, priceQueue] : book)
    {
        if (!order->CanMatch(price))
            return order->GetCount() == 0;

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
    }
    return order->GetCount() == 0;
}

bool OrderBook::Match(std::shared_ptr<Order> order)
{
    assert(order->GetActivated() == false);
    std::unique_lock<std::mutex> l(order->GetSide() == Side::BUY ? asks_lock : bids_lock);

    if (order->GetSide() == Side::BUY)
        return CrossSpread(asks, order, l);
    else
        return CrossSpread(bids, order, l);
}

void OrderBook::Execute(std::shared_ptr<Order> order)
{
    // Perform CrossSpread and match orders to execute
    bool filled = Match(order);

    std::unique_lock<std::mutex> lo(order->GetSide() == Side::BUY ? bids_lock : asks_lock);

    if (!filled)
        Output::OrderAdded(
            order->GetOrderId(),
            order->GetInstrumentId().c_str(),
            order->GetPrice(),
            order->GetCount(),
            order->GetSide() == Side::SELL,
            getCurrentTimestamp());
    // Add
    order->Activate();
}

void OrderBook::Cancel(std::shared_ptr<Order> order)
{
    std::unique_lock<std::mutex> l(order->GetSide() == Side::BUY ? bids_lock : asks_lock);
    while (!order->GetActivated())
        order->cv.wait(l);

    std::shared_ptr<Price> priceQueue = GetPrice(order->GetSide(), order->GetPrice());
    Price::iterator start;
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

// TODO: Refactor into method in Book class
template <typename T>
std::shared_ptr<Price> GetOrAssign(std::map<price_t, std::shared_ptr<Price>, T> & map, price_t price)
{
    if (!map.contains(price))
        map[price] = std::make_shared<Price>();
    return map[price];
}

std::shared_ptr<Price> OrderBook::GetPrice(Side side, price_t price)
{
    if (side == Side::BUY)
        return GetOrAssign(bids, price);
    else
        return GetOrAssign(asks, price);
}
