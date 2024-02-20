#ifndef _ORDER_BOOK_HPP_
#define _ORDER_BOOK_HPP_
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>

#include "Order.hpp"
#include "Price.hpp"

auto BidsComparator = [](const Price &x, const Price &y)
{
  return x.GetPrice() > y.GetPrice();
};

auto AsksComparator = [](const Price &x, const Price &y)
{
  return x.GetPrice() < y.GetPrice();
};

class OrderBook
{
public:
  OrderBook() = default;
  void AddOrder(Order &order);
  bool ExecuteOrder(Order &order);

private:
  struct OrderBookEntry
  {
    std::priority_queue<Price, std::vector<Price>, decltype(BidsComparator)> bids;
    std::priority_queue<Price, std::vector<Price>, decltype(AsksComparator)> asks;
  };
  std::unordered_map<order_id_t, struct OrderBookEntry> order_book;
};

#endif