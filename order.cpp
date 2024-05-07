
#include "order.hpp"

Order::Order(
    order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count, Side side, std::chrono::microseconds::rep timestamp)
    : order_id(order_id)
    , execution_id(0)
    , instrument(instrument)
    , price(price)
    , count(count)
    , side(side)
    , timestamp(timestamp)
    , activated(false)
    , completed(false)
{
}
