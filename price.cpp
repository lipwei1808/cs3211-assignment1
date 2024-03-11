#include <memory>
#include <queue>
#include "price.hpp"
#include "order.hpp"

Price::Price(unsigned int price) : orders(std::queue<std::shared_ptr<Order>>()), price(price) {}

void Price::AddOrder(std::shared_ptr<Order> order)
{
  orders.push(order);
}

std::shared_ptr<Order> Price::Front()
{
  if (orders.size() == 0)
  {
    return nullptr;
  }

  std::shared_ptr<Order> order = orders.front();
  return order;
}

void Price::Pop()
{
  if (orders.size() > 0)
  {
    orders.pop();
  }
}

size_t Price::Size() const
{
  return orders.size();
}

unsigned int Price::GetPrice() const
{
  return price;
}