
#include "order.hpp"

Order::Order(
    unsigned int order_id, std::string instrument, unsigned int price,
    unsigned int count, Side side, std::chrono::microseconds::rep timestamp)
    : order_id(order_id),
      execution_id(0),
      instrument(instrument),
      price(price),
      count(count),
      side(side),
      timestamp(timestamp),
      activated(false) {}
