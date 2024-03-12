#ifndef PRICE_HPP
#define PRICE_HPP
#include <queue>
#include <memory>
#include <mutex>
#include "order.hpp"

class Price
{
public:
  Price();
  void AddOrder(std::shared_ptr<Order> order);
  void Pop();
  std::shared_ptr<Order> Front();
  size_t Size() const;

private:
  std::queue<std::shared_ptr<Order>> orders;
  std::mutex lock;
};

#endif