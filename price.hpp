#ifndef PRICE_HPP
#define PRICE_HPP
#include <queue>
#include <memory>
#include <mutex>
#include "order.hpp"

class Price
{
public:
  Price(unsigned int price);
  void AddOrder(std::shared_ptr<Order> order);
  void Pop();
  std::shared_ptr<Order> Front();
  size_t Size() const;
  unsigned int GetPrice() const;

private:
  std::queue<std::shared_ptr<Order>> orders;
  unsigned int price;
  std::mutex lock;
};

#endif