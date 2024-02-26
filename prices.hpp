#ifndef _PRICES_HPP_
#define _PRICES_HPP_
#include <map>
#include <shared_mutex>
#include <memory>

#include "price.hpp"
#include "order.hpp"

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

class Prices
{
public:
  std::shared_ptr<Price> GetPrice(price_t price);
  std::shared_ptr<Price> CreatePrice(price_t price);

private:
  std::map<price_t, std::shared_ptr<Price>, PriceComparator> prices;
  std::shared_mutex lock;
};

#endif