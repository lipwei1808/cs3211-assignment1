#ifndef ORDER_HPP
#define ORDER_HPP

#include <condition_variable>
#include <mutex>
#include <string>

#include "io.hpp"

typedef unsigned int order_id_t;
typedef unsigned int execution_id_t;
typedef std::string instrument_id_t;
typedef unsigned int price_t;

enum class Side
{
    BUY,
    SELL
};

/**
 * Represents a Buy or Sell Order.
*/
class Order
{
public:
    /**
     * Factory method to create an Order based on the side.
    */
    static std::shared_ptr<Order> from(order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count, Side side);
    order_id_t GetOrderId() const { return order_id; }
    execution_id_t GetExecutionId() const { return execution_id; }
    void IncrementExecutionId() { execution_id++; }
    instrument_id_t GetInstrumentId() const { return instrument; }
    price_t GetPrice() const { return price; }
    unsigned int GetCount() const { return count; }
    std::chrono::microseconds::rep GetTimestamp() { return timestamp; }
    void SetTimestamp(std::chrono::microseconds::rep tm) { timestamp = tm; }
    bool GetActivated() { return activated; }
    bool GetCompleted() const { return completed; }
    void SetCompleted() { completed = true; }
    void Fill(unsigned int qty) { count = qty >= count ? 0 : count - qty; }

    virtual Side GetSide() const = 0;
    virtual bool CanMatch(price_t price) = 0;
    virtual ~Order() = default;

    void Activate()
    {
        activated = true;
        cv.notify_all();
    }

    std::condition_variable cv;

protected:
    Order(order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count);

private:
    order_id_t order_id;
    execution_id_t execution_id;
    instrument_id_t instrument;
    price_t price;
    unsigned int count;
    std::chrono::microseconds::rep timestamp;
    bool activated;
    bool completed;
};

class BuyOrder : public Order
{
public:
    BuyOrder(order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count) : Order(order_id, instrument, price, count)
    {
    }
    virtual Side GetSide() const override { return Side::BUY; }
    virtual bool CanMatch(price_t price) override { return GetPrice() >= price; }
    virtual ~BuyOrder() = default;
};
class SellOrder : public Order
{
public:
    SellOrder(order_id_t order_id, instrument_id_t instrument, price_t price, unsigned int count)
        : Order(order_id, instrument, price, count)
    {
    }

    virtual Side GetSide() const override { return Side::SELL; }
    virtual bool CanMatch(price_t price) override { return GetPrice() <= price; }
    virtual ~SellOrder() = default;
};

#endif