
#include "order.hpp"

Order::Order(
    unsigned int order_id, std::string instrument, unsigned int price,
    unsigned int count, Side side, std::chrono::microseconds::rep timestamp)
    : order_id(order_id),
      instrument(instrument),
      price(price),
      count(count),
      side(side),
      timestamp(timestamp) {}

Side Order::GetSide() const
{
  return side;
}

instrument_id_t Order::GetInstrumentId() const
{
  return instrument;
}