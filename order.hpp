#ifndef _ORDER_HPP_
#define _ORDER_HPP_

#include <string>

class Order
{
public:
  Order(unsigned int order_id, std::string instrument, double price, unsigned int count, std::string side, std::tm timestamp);
private:
  unsigned int order_id;
  std::string instrument;
  double price;
  unsigned int count;
  std::string side; // TODO: enum
  std::tm timestamp;
};

#endif