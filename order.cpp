
#include "order.hpp"

Order::Order(
  unsigned int order_id,
  std::string instrument,
  double price, 
  unsigned int count, 
  std::string side, 
  std::tm timestamp
): order_id(order_id), instrument(instrument), price(price), count(count) , side(side), timestamp(timestamp) {

}