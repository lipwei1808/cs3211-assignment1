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
    GetBook(order->GetSide())->Add(order);
}

void OrderBook::Execute(std::shared_ptr<Order> order)
{
    // Perform CrossSpread and match orders to execute
    bool filled = GetOtherBook(order->GetSide())->CrossSpread(order);

    GetBook(order->GetSide())->AfterExecute(order, filled);
}

void OrderBook::Cancel(std::shared_ptr<Order> order)
{
    GetBook(order->GetSide())->Cancel(order);
}

BaseBook * OrderBook::GetBook(Side side)
{
    if (side == Side::BUY)
        return &bids;
    else
        return &asks;
}

BaseBook * OrderBook::GetOtherBook(Side side)
{
    return GetBook(side == Side::BUY ? Side::SELL : Side::BUY);
}