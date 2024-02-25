#ifndef _ORDER_HPP_
#define _ORDER_HPP_

#include <string>

typedef unsigned int order_id_t;
typedef std::string instrument_id_t;
typedef unsigned int price_t;

enum class Side
{
  BUY,
  SELL
};

class Order
{
public:
  Order(unsigned int order_id, std::string instrument, unsigned int price, unsigned int count, Side side, std::chrono::microseconds::rep timestamp);
  Side GetSide() const;
  instrument_id_t GetInstrumentId() const;

private:
  order_id_t order_id;
  instrument_id_t instrument;
  unsigned int price;
  unsigned int count;
  Side side; // TODO: enum
  std::chrono::microseconds::rep timestamp;
};

#endif