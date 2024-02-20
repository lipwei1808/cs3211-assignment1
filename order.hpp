#ifndef _ORDER_HPP_
#define _ORDER_HPP_

#include <string>

typedef unsigned int order_id_t;

class Order
{
public:
  Order(unsigned int order_id, std::string instrument, unsigned int price, unsigned int count, std::string side, std::tm timestamp);

private:
  order_id_t order_id;
  std::string instrument;
  unsigned int price;
  unsigned int count;
  std::string side; // TODO: enum
  std::tm timestamp;
};

#endif