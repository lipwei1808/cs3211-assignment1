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
  void HandleBuy(std::shared_ptr<Order> order);
  void HandleSell(std::shared_ptr<Order> order);
  void AddBuy(std::shared_ptr<Order> order);
  void AddSell(std::shared_ptr<Order> order);
  void ExecuteBuy(std::shared_ptr<Order> order);
  void ExecuteSell(std::shared_ptr<Order> order);

  AtomicMap<price_t, std::shared_ptr<Price>, std::greater<price_t>> bids;
  AtomicMap<price_t, std::shared_ptr<Price>> asks;

  std::mutex order_book_lock;
  std::mutex bids_lock;
  std::mutex asks_lock;
};

#endif