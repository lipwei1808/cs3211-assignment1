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

class Order
{
public:
    Order(
        unsigned int order_id,
        std::string instrument,
        unsigned int price,
        unsigned int count,
        Side side,
        std::chrono::microseconds::rep timestamp);
    order_id_t GetOrderId() const { return order_id; }
    execution_id_t GetExecutionId() const { return execution_id; }
    void IncrementExecutionId() { execution_id++; }
    instrument_id_t GetInstrumentId() const { return instrument; }
    price_t GetPrice() const { return price; }
    unsigned int GetCount() const { return count; }
    Side GetSide() const { return side; }
    std::chrono::microseconds::rep GetTimestamp() { return timestamp; }

    void SetTimestamp(std::chrono::microseconds::rep tm) { timestamp = tm; }

    bool GetActivated()
    {
        std::unique_lock<std::mutex> l(lock);
        return activated;
    }

    void Fill() { Fill(count); }

    void Fill(unsigned int qty) { count = qty >= count ? 0 : count - qty; }

    void Activate()
    {
        std::unique_lock<std::mutex> l(lock);
        activated = true;
        cv.notify_all();
    }

    std::condition_variable cv;

private:
    order_id_t order_id;
    execution_id_t execution_id;
    instrument_id_t instrument;
    price_t price;
    unsigned int count;
    Side side;
    std::chrono::microseconds::rep timestamp;
    bool activated;

    std::mutex lock;
};

#endif