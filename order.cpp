
#include "order.hpp"

Order::Order(order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count, std::chrono::microseconds::rep timestamp)
    : order_id(order_id)
    , execution_id(0)
    , instrument(instrument)
    , price(price)
    , count(count)
    , timestamp(timestamp)
    , activated(false)
    , completed(false)
{
}

std::shared_ptr<Order> Order::from(
    order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count, Side side, std::chrono::microseconds::rep timestamp)
{
    if (side == Side::BUY)
        return std::make_shared<BuyOrder>(order_id, instrument, price, count, timestamp);
    else
        return std::make_shared<SellOrder>(order_id, instrument, price, count, timestamp);
}