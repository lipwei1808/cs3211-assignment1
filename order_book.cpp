#include <memory>

#include "order.hpp"
#include "order_book.hpp"

bool OrderBook::ExecuteOrder(std::shared_ptr<Order> order)
{
  struct OrderBookEntry obe = GetOrderBookEntry(order->GetInstrumentId());
  switch (order->GetSide())
  {
  case Side::BUY:
  {
    if (MatchBuy(order))
    {
      return true;
    }
    return true;
  }
  case Side::SELL:
  {
    if (MatchSell(order))
    {
      return true;
    }
    return true;
  }
  default:
  {
    return false;
  }
  }
}

bool OrderBook::AddOrder(std::shared_ptr<Order> order, std::shared_ptr<Price>)
{
  return true;
}

/**
 * Search the asks OrderBookEntry and check if there is
 * a price lower or equals than the buy price.
 */
bool OrderBook::MatchBuy(std::shared_ptr<Order> order)
{
  return true;
}

bool OrderBook::MatchSell(std::shared_ptr<Order> order)
{
  return true;
}

OrderBook::OrderBookEntry &OrderBook::GetOrderBookEntry(instrument_id_t instrument)
{
  if (order_book.find(instrument) == order_book.end())
  {
    OrderBookEntry obe({.bids = Prices(Side::BUY), .asks = Prices(Side::SELL), .bids_mutex = std::mutex(), .asks_mutex = std::mutex()});
    order_book.insert({instrument, obe});
  }
  return order_book[instrument];
}

std::shared_ptr<Price> OrderBook::GetPrice(price_t price, Prices &prices)
{
  if (prices.find(price) == prices.end())
  {
  }
  return prices[price];
}