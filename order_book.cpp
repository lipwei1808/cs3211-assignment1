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
  std::unique_lock<std::mutex> l(order_book_lock);

  // Get timestamp for order
  order.SetTimestamp(getCurrentTimestamp());

  // Insert dummy node into order
  bool res Add(order, bids);

  l.unlock();
  // Execute
  if (ExecuteBuy(order))
  {
    return true;
  }

  // Add
  return Add(order, bids);
}

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}