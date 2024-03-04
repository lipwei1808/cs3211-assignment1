#include <assert.h>

#include "order_book.hpp"
#include "order.hpp"

bool OrderBook::HandleOrder(Order &order)
{
  switch (order.GetSide())
  {
  case Side::BUY:
    return HandleBuy(order);
  case Side::SELL:
    return HandleSell(order);
  default:
    return false;
  }
}

bool OrderBook::HandleBuy(Order &order)
{
  assert(order.GetSide() == Side::BUY);
  assert(order.GetActivated() == false);
  std::unique_lock<std::mutex> l(order_book_lock);

  // Get timestamp for order
  order.SetTimestamp(getCurrentTimestamp());

  // Insert dummy node into order
  bool res = AddBuy(order);

  l.unlock();

  // Execute
  if (ExecuteBuy(order))
  {
    return true;
  }

  // Add
  order.Activate();
  return true;
}

bool OrderBook::AddBuy(Order &order)
{
  assert(order.GetSide() == Side::BUY);
  std::unique_lock<std::mutex> l(bids_lock);
  std::shared_ptr<Price> p = bids.Get(order.GetPrice());
  p->AddOrder(std::make_shared<Order>(order));
  return true;
}

bool OrderBook::ExecuteBuy(Order &order)
{
  assert(order.GetSide() == Side::BUY);
  assert(order.GetActivated() == false);
  std::unique_lock<std::mutex> l(asks_lock);

  while (order.GetCount() > 0)
  {
    // Check if any sell orders
    if (asks.Size() == 0)
    {
      return false;
    }

    // Check if sell order is able to match
    auto firstEl = asks.begin();
    if (firstEl->first > order.GetPrice())
    {
      return false;
    }

    std::shared_ptr<Price> priceQueue = firstEl->second;
    assert(priceQueue->Size() != 0);

    // Check if sell order is activated
    while (order.GetCount() > 0 && priceQueue->Size())
    {
      std::shared_ptr<Order> sellOrder = priceQueue->Front();
      while (!sellOrder->GetActivated())
      {
        sellOrder->cv.wait(l);
      }

      // Check if dummy order has already been filled
      if (sellOrder->GetCount() == 0)
      {
        priceQueue->Pop();
        continue;
      }

      if (sellOrder->GetCount() == order.GetCount())
      {
        priceQueue->Pop();
        order.Fill();
      }
      else if (sellOrder->GetCount() > order.GetCount())
      {
        sellOrder->Fill(order.GetCount());
        order.Fill();
      }
      else
      {
        order.Fill(sellOrder->GetCount());
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

  return order.GetCount() == 0;
}

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}