#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <memory>

#include "atomic_map.hpp"
#include "order.hpp"
#include "price.hpp"

struct PriceComparator
{
  Side side;
  PriceComparator(Side side) : side(side) {}
  bool operator()(const std::shared_ptr<Price> &x, const std::shared_ptr<Price> &y)
  {
    switch (side)
    {
    case Side::BUY:
    {
      return x->GetPrice() > y->GetPrice();
    }
    case Side::SELL:
    {
      return x->GetPrice() < y->GetPrice();
    }
    default:
    {
      throw std::runtime_error("price comparator fail");
    }
    }
  }
};

class OrderBook
{
public:
  OrderBook() = default;
  bool ExecuteOrder(Order &order);
  bool CancelOrder(Order &order);

private:
  AtomicMap<price_t, std::shared_ptr<Price>, PriceComparator> bids;
  AtomicMap<price_t, std::shared_ptr<Price>, PriceComparator> asks;
};

#endif