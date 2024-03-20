#include <assert.h>
#include <memory>
#include <mutex>
#include <iostream>
#include <algorithm>
#include <string>

#include "order_book.hpp"
#include "order.hpp"

template void OrderBook::Handle<Side::BUY>(std::shared_ptr<Order> order);
template void OrderBook::Handle<Side::SELL>(std::shared_ptr<Order> order);

template <Side side>
void OrderBook::Handle(std::shared_ptr<Order> order)
{
  assert(side == Side::SELL || side == Side::BUY);
  assert(order->GetSide() == side);
  assert(order->GetActivated() == false);
  std::unique_lock<std::mutex> l(order_book_lock);

  // Get timestamp for order
  order->SetTimestamp(getCurrentTimestamp());

  // Insert dummy node into order
  if constexpr (side == Side::BUY)
    Add<Side::BUY>(order);
  else
    Add<Side::SELL>(order);

  l.unlock();

  // Execute
  bool filled = side == Side::BUY ? ExecuteBuy(order) : ExecuteSell(order);
  if (!filled)
  {
    Output::OrderAdded(
        order->GetOrderId(),
        order->GetInstrumentId().c_str(),
        order->GetPrice(),
        order->GetCount(),
        side == Side::SELL,
        order->GetTimestamp());
  }

  // Add
  order->Activate();
}

/**
 * @param order active buy order to execute.
 * @return if buy order has been fully completed.
 */
bool OrderBook::ExecuteBuy(std::shared_ptr<Order> order)
{
  assert(order->GetSide() == Side::BUY);
  assert(order->GetActivated() == false);
  std::unique_lock<std::mutex> l(asks_lock);
  while (order->GetCount() > 0)
  {
    // Check if any sell orders
    if (asks.Size() == 0)
      return false;

    // Check if lowest sell order can match the buy
    auto firstEl = asks.begin();
    price_t price = firstEl->first;
    assert(firstEl->second.initialised);
    std::shared_ptr<Price> priceQueue = firstEl->second.Get();
    assert(priceQueue->Size() != 0);
    if (price > order->GetPrice())
      return false;

    // Iteratively match with all orders in this price queue.
    while (order->GetCount() > 0 && priceQueue->Size())
    {
      std::shared_ptr<Order> sellOrder = priceQueue->Front();
      // Check if first order's timestamp comes before the current buy
      if (sellOrder->GetTimestamp() > order->GetTimestamp())
        break;

      // Check if sell order is activated
      while (!sellOrder->GetActivated())
        sellOrder->cv.wait(l);

      // Check if dummy order has already been filled
      if (sellOrder->GetCount() == 0)
      {
        priceQueue->Pop();
        continue;
      }
      sellOrder->IncrementExecutionId();
      MatchOrders(order, sellOrder);
      if (sellOrder->GetCount() == 0)
      {
        priceQueue->Pop();
      }
    }

    if (priceQueue->Size() == 0)
    {
      // Remove from map of prices
      size_t num = asks.Erase(firstEl->first);
      assert(num == 1);
    }
  }

  return order->GetCount() == 0;
}

/**
 * @param order active buy order to execute.
 * @return if buy order has been fully completed.
 */
bool OrderBook::ExecuteSell(std::shared_ptr<Order> order)
{
  assert(order->GetSide() == Side::SELL);
  assert(order->GetActivated() == false);
  std::unique_lock<std::mutex> l(bids_lock);

  while (order->GetCount() > 0)
  {
    // Check if any sell orders
    if (bids.Size() == 0)
      return false;

    // Check if lowest sell order can match the buy
    auto firstEl = bids.begin();
    price_t price = firstEl->first;
    assert(firstEl->second.initialised);
    std::shared_ptr<Price> priceQueue = firstEl->second.Get();
    assert(priceQueue->Size() != 0);
    if (price < order->GetPrice())
      return false;

    // Iteratively match with all orders in this price queue.
    while (order->GetCount() > 0 && priceQueue->Size())
    {
      std::shared_ptr<Order> oppOrder = priceQueue->Front();
      // Check if first order's timestamp comes before the current buy
      if (oppOrder->GetTimestamp() > order->GetTimestamp())
        break;

      // Check if sell order is activated
      while (!oppOrder->GetActivated())
        oppOrder->cv.wait(l);

      // Check if dummy order has already been filled
      if (oppOrder->GetCount() == 0)
      {
        priceQueue->Pop();
        continue;
      }

      oppOrder->IncrementExecutionId();
      MatchOrders(order, oppOrder);
      if (oppOrder->GetCount() == 0)
      {
        priceQueue->Pop();
      }

      if (priceQueue->Size() == 0)
      {
        // Remove from map of prices
        size_t num = bids.Erase(firstEl->first);
        assert(num == 1);
      }
    }
  }
  return order->GetCount() == 0;
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
      resting->GetOrderId(),
      incoming->GetOrderId(),
      resting->GetExecutionId(),
      resting->GetPrice(),
      qty,
      getCurrentTimestamp());
}