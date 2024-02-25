#include <memory>

#include "order.hpp"
#include "order_book.hpp"

bool OrderBook::ExecuteOrder(std::shared_ptr<Order> order)
{
  instrument_id_t instrument = order->GetInstrumentId();
  if (order_book.find(instrument) == order_book.end())
  {
    order_book[instrument] = OrderBookEntry();
  }
  struct OrderBookEntry obe = order_book[instrument];
  switch (order->GetSide())
  {
  case Side::BUY:
  {
    if (MatchBuy(order))
    {
      return true;
    }
    return AddOrder(order);
  }
  case Side::SELL:
  {
    if (MatchSell(order))
    {
      return true;
    }
    return AddOrder(order);
  }
  default:
  {
    return false;
  }
  }
}

bool OrderBook::AddOrder(std::shared_ptr<Order> order, std::shared_ptr<Price>) {}

/**
 * Search the asks OrderBookEntry and check if there is
 * a price lower or equals than the buy price.
 */
bool OrderBook::MatchBuy(std::shared_ptr<Order> order)
{
}

bool OrderBook::MatchSell(std::shared_ptr<Order> order) {}

OrderBook::OrderBookEntry OrderBook::GetOrderBookEntry(instrument_id_t instrument)
{
  if (order_book.find(instrument) == order_book.end())
  {
    OrderBookEntry obe({.bids = Prices(Side::BUY), .asks = Prices(Side::SELL)});
    order_book.insert({instrument, obe});
  }
  return order_book[instrument];
}