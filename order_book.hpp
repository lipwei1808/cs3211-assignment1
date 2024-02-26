#ifndef _ORDER_BOOK_HPP_
#define _ORDER_BOOK_HPP_
#include <unordered_map>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <memory>

#include "order.hpp"
#include "price.hpp"
#include "prices.hpp"

class OrderBook
{
public:
  OrderBook() = default;
  void CancelOrder(std::shared_ptr<Order> order);
  bool ExecuteOrder(std::shared_ptr<Order> order);

private:
  struct OrderBookEntry
  {
    Prices bids;
    Prices asks;
  };
  bool MatchBuy(std::shared_ptr<Order> order);
  bool MatchSell(std::shared_ptr<Order> order);
  bool AddOrder(std::shared_ptr<Order> order, Prices &prices);
  OrderBookEntry &GetOrderBookEntry(instrument_id_t instrument);
  std::shared_ptr<Price> GetPrice(price_t price, Prices &prices);

  std::unordered_map<instrument_id_t, struct OrderBookEntry> order_book;
};

#endif