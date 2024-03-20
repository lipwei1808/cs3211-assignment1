#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <mutex>
#include <chrono>
#include <memory>
#include <assert.h>

#include "atomic_map.hpp"
#include "order.hpp"
#include "price.hpp"

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// TODO: Check if bids_lock/asks_lock required when using AtomicMap
class OrderBook
{
public:
  OrderBook() = default;
  template <Side side>
  void Handle(std::shared_ptr<Order> order);

private:
  template <Side side>
  void Add(std::shared_ptr<Order> order);
  template <Side side>
  std::shared_ptr<Price> GetPrice(price_t price);
  template <Side side>
  bool Execute(std::shared_ptr<Order> order);
  void MatchOrders(std::shared_ptr<Order> o1, std::shared_ptr<Order> o2);

  AtomicMap<price_t, WrapperValue<std::shared_ptr<Price>>, std::greater<price_t>> bids;
  AtomicMap<price_t, WrapperValue<std::shared_ptr<Price>>> asks;

  std::mutex order_book_lock;
  std::mutex bids_lock;
  std::mutex asks_lock;
};

#endif