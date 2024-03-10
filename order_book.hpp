#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <mutex>
#include <chrono>
#include <memory>
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
  bool HandleBuy(std::shared_ptr<Order> order);

  bool HandleSell(std::shared_ptr<Order> order)
  {
    assert(order->GetSide() == Side::BUY);
    std::unique_lock<std::mutex> l(order_book_lock);
    return true;
  }

  bool ExecuteBuy(std::shared_ptr<Order> order);

  bool ExecuteSell(Order &order)
  {

    assert(order.GetSide() == Side::SELL);
    std::unique_lock<std::mutex> l(order_book_lock);
    return true;
  }

  bool AddBuy(std::shared_ptr<Order> order);

  AtomicMap<price_t, std::shared_ptr<Price>, std::greater<price_t>> bids;
  AtomicMap<price_t, std::shared_ptr<Price>> asks;

  std::mutex order_book_lock;
  std::mutex bids_lock;
  std::mutex asks_lock;
};

#endif