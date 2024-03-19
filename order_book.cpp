#include <assert.h>
#include <memory>
#include <mutex>
#include <iostream>
#include <algorithm>
#include <string>

#include "order_book.hpp"
#include "order.hpp"

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool OrderBook::HandleOrder(std::shared_ptr<Order> order)
{
  switch (order->GetSide())
  {
  case Side::BUY:
    HandleBuy(order);
    break;
  case Side::SELL:
    HandleSell(order);
    break;
  default:
    return false;
  }

  return true;
}

void OrderBook::HandleBuy(std::shared_ptr<Order> order)
{
  assert(order->GetSide() == Side::BUY);
  assert(order->GetActivated() == false);
  std::unique_lock<std::mutex> l(order_book_lock);

  // Get timestamp for order
  order->SetTimestamp(getCurrentTimestamp());

  // Insert dummy node into order
  AddBuy(order);

  l.unlock();

  // Execute
  bool filled = ExecuteBuy(order);
  if (!filled)
  {
    Output::OrderAdded(
        order->GetOrderId(),
        order->GetInstrumentId().c_str(),
        order->GetPrice(),
        order->GetCount(),
        false,
        order->GetTimestamp());
  }
  // Add
  order->Activate();
}

void OrderBook::HandleSell(std::shared_ptr<Order> order)
{
  assert(order->GetSide() == Side::SELL);
  assert(order->GetActivated() == false);
  std::unique_lock<std::mutex> l(order_book_lock);

  // Get timestamp for order
  order->SetTimestamp(getCurrentTimestamp());

  // Insert dummy node into order
  AddSell(order);

  l.unlock();

  // Execute
  bool filled = ExecuteSell(order);
  if (!filled)
  {
    Output::OrderAdded(
        order->GetOrderId(),
        order->GetInstrumentId().c_str(),
        order->GetPrice(),
        order->GetCount(),
        true,
        order->GetTimestamp());
  }

  // Add
  order->Activate();
}

void OrderBook::AddBuy(std::shared_ptr<Order> order)
{
  assert(order->GetSide() == Side::BUY);
  std::unique_lock<std::mutex> l(bids_lock);
  std::shared_ptr<Price> p = GetPrice(bids, order->GetPrice());
  p->AddOrder(order);
}

void OrderBook::AddSell(std::shared_ptr<Order> order)
{
  assert(order->GetSide() == Side::SELL);
  std::unique_lock<std::mutex> l(asks_lock);
  std::shared_ptr<Price> p = GetPrice(asks, order->GetPrice());
  p->AddOrder(order);
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
  std::cout << "[ExecuteBuy] Order count: " << order->GetCount() << ", Asks map size: " << asks.Size() << std::endl;
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
    if (price > order->GetPrice())
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