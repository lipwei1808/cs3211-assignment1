#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <mutex>
#include <chrono>
#include <assert.h>

#include "atomic_map.hpp"
#include "order.hpp"
#include "price.hpp"

class OrderBook
{
public:
  OrderBook() = default;
  bool HandleOrder(Order &order);

private:
  bool HandleBuy(Order &order);

  bool HandleSell(Order &order)
  {
    assert(order.side == Side::BUY);
    std::unique_lock<std::mutex> l(order_book_lock);
    return true;
  }

  bool ExecuteBuy(Order &order)
  {
    assert(order.side == Side::BUY);
    std::unique_lock<std::mutex> l(order_book_lock);
    return true;
  }
  bool ExecuteSell(Order &order)
  {

    assert(order.side == Side::SELL);
    std::unique_lock<std::mutex> l(order_book_lock);
    return true;
  }

  template <typename S>
  bool Add(Order &order, AtomicMap<price_t, std::shared_ptr<Price>, S> &map)
  {

    assert(order.GetSide() == Side::BUY);
    std::unique_lock<std::mutex> l(bids_lock);
    return true;
  }

  AtomicMap<price_t, std::shared_ptr<Price>, std::greater<price_t>> bids;
  AtomicMap<price_t, std::shared_ptr<Price>> asks;

  std::mutex order_book_lock;
  std::mutex bids_lock;
  std::mutex asks_lock;
};

#endif