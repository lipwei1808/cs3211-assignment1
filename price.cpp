#include <memory>
#include "price.hpp"
#include "order.hpp"

Price::Price(unsigned int price) : price(price) {}

bool Price::AddOrder(std::shared_ptr<Order> order)
{
  orders.push(order);
}

std::shared_ptr<Order> Price::PopOrder()
{
  if (orders.size() == 0)
  {
    return nullptr;
  }

  std::shared_ptr<Order> order = orders.front();
  orders.pop();
  return order;
}

size_t Price::Size() const
{
  return orders.size();
}

unsigned int Price::GetPrice() const
{
  return price;
}