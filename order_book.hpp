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

class OrderBook
{
public:
  OrderBook() = default;
  template <Side side>
  void Handle(std::shared_ptr<Order> order);

private:
  void AddBuy(std::shared_ptr<Order> order);
  void AddSell(std::shared_ptr<Order> order);
  bool ExecuteBuy(std::shared_ptr<Order> order);
  bool ExecuteSell(std::shared_ptr<Order> order);
  void MatchOrders(std::shared_ptr<Order> o1, std::shared_ptr<Order> o2);
  template <typename T>
  std::shared_ptr<Price> GetPrice(AtomicMap<price_t, WrapperValue<std::shared_ptr<Price>>, T> &map, price_t price)
  {
    WrapperValue<std::shared_ptr<Price>> &w = map.Get(price);
    std::unique_lock<std::mutex> l(w.lock);
    if (!w.initialised)
    {
      w.initialised = true;
      w.val = std::make_shared<Price>();
    }
    return w.val;
  }

  AtomicMap<price_t, WrapperValue<std::shared_ptr<Price>>, std::greater<price_t>> bids;
  AtomicMap<price_t, WrapperValue<std::shared_ptr<Price>>> asks;

  std::mutex order_book_lock;
  std::mutex bids_lock;
  std::mutex asks_lock;
};

#endif