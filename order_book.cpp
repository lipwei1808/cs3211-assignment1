#include <algorithm>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <assert.h>

#include "book.hpp"
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
    if (order->GetSide() == Side::BUY)
        bids.Add(order);
    else
        asks.Add(order);
}

bool OrderBook::Match(std::shared_ptr<Order> order)
{
    assert(order->GetActivated() == false);

    if (order->GetSide() == Side::BUY)
        return asks.CrossSpread(order);
    else
        return bids.CrossSpread(order);
}

void OrderBook::Execute(std::shared_ptr<Order> order)
{
    // Perform CrossSpread and match orders to execute
    bool filled = Match(order);

    if (order->GetSide() == Side::BUY)
        bids.AfterExecute(order, filled);
    else
        asks.AfterExecute(order, filled);
}

void OrderBook::Cancel(std::shared_ptr<Order> order)
{
    if (order->GetSide() == Side::BUY)
        bids.Cancel(order);
    else
        asks.Cancel(order);
}
