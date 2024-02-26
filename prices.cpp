
#include <memory>
#include <shared_mutex>
#include "prices.hpp"
#include "price.hpp"

std::shared_ptr<Price> Prices::GetPrice(price_t price)
{
  std::shared_lock<std::shared_mutex> ul(lock);
  if (prices.find(price) == prices.end())
  {
    ul.unlock();
    return CreatePrice(price);
  }

  return prices[price];
}

std::shared_ptr<Price> Prices::CreatePrice(price_t price)
{
  std::unique_lock<std::shared_mutex> ul(lock);
  std::shared_ptr<Price> p = std::make_shared<Price>(price);
  prices.insert({price, p});
  return p;
}