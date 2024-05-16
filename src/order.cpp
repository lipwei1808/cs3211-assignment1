
#include "order.hpp"

Order::Order(order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count)
    : order_id(order_id)
    , execution_id(0)
    , instrument(instrument)
    , price(price)
    , count(count)
    , timestamp(0)
    , activated(false)
    , completed(false)
{
}

std::shared_ptr<Order> Order::from(order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count, Side side)
{
    if (side == Side::BUY)
        return std::make_shared<BuyOrder>(order_id, instrument, price, count);
    else
        return std::make_shared<SellOrder>(order_id, instrument, price, count);
}