#ifndef _ORDER_BOOK_HPP_
#define _ORDER_BOOK_HPP_
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <memory>

#include "order.hpp"
#include "price.hpp"

struct PriceComparator
{
  Side side;
  PriceComparator(Side side) : side(side) {}
  bool operator()(const Price &x, const Price &y)
  {
    switch (side)
    {
    case Side::BUY:
    {
      return x.GetPrice() > y.GetPrice();
    }
    case Side::SELL:
    {
      return x.GetPrice() > y.GetPrice();
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
  void CancelOrder(std::shared_ptr<Order> order);
  bool ExecuteOrder(std::shared_ptr<Order> order);

private:
  using Prices = std::map<price_t, std::shared_ptr<Price>, PriceComparator>;
  struct OrderBookEntry
  {
    Prices bids;
    Prices asks;
  };
  bool MatchBuy(std::shared_ptr<Order> order);
  bool MatchSell(std::shared_ptr<Order> order);
  bool AddOrder(std::shared_ptr<Order> order, std::shared_ptr<Price>);
  OrderBookEntry GetOrderBookEntry(instrument_id_t instrument);

  std::unordered_map<instrument_id_t, struct OrderBookEntry> order_book;
};

#endif