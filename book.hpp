#ifndef BOOK_HPP
#define BOOK_HPP

#include <deque>
#include <map>
#include <memory>
#include <mutex>

#include "order.hpp"

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

inline void MatchOrders(std::shared_ptr<Order> incoming, std::shared_ptr<Order> resting)
{
    unsigned int qty = std::min(incoming->GetCount(), resting->GetCount());
    incoming->Fill(qty);
    resting->Fill(qty);
    Output::OrderExecuted(
        resting->GetOrderId(), incoming->GetOrderId(), resting->GetExecutionId(), resting->GetPrice(), qty, getCurrentTimestamp());
}

typedef std::deque<std::shared_ptr<Order>> Price;
template <typename T>
using BookT = std::map<price_t, std::shared_ptr<Price>, T>;

class BaseBook
{
public:
    virtual void Add(std::shared_ptr<Order> order) = 0;
    virtual void Cancel(std::shared_ptr<Order> order) = 0;
    virtual void AfterExecute(std::shared_ptr<Order> order, bool filled) = 0;
    virtual bool CrossSpread(std::shared_ptr<Order> order) = 0;
    virtual ~BaseBook() = default;
};

template <typename T>
class Book : public BaseBook
{
public:
    virtual void Add(std::shared_ptr<Order> order) override
    {
        std::unique_lock<std::mutex> l(mutex);
        std::shared_ptr<Price> p = GetOrAssign(order->GetPrice());
        p->push_back(order);
    }

    virtual bool CrossSpread(std::shared_ptr<Order> order) override
    {
        std::unique_lock<std::mutex> l(mutex);
        if (map.size() == 0)
            return false;

        for (auto & [price, priceQueue] : map)
        {
            if (!order->CanMatch(price))
                return order->GetCount() == 0;

            // Iteratively match with all orders in this price queue.
            while (order->GetCount() > 0 && priceQueue->size())
            {
                std::shared_ptr<Order> oppOrder = priceQueue->front();
                // Check if first order's timestamp comes before the current buy
                if (oppOrder->GetTimestamp() > order->GetTimestamp())
                    break;

                // Check if sell order is activated
                while (!oppOrder->GetActivated())
                {
                    SyncInfo() << "[EXECUTE] Order: " << order->GetOrderId() << " going to sleep" << std::endl;
                    oppOrder->cv.wait(l);
                }

                if (oppOrder->GetCompleted())
                    continue;

                // Check if dummy order has already been filled
                if (oppOrder->GetCount() > 0)
                {
                    oppOrder->IncrementExecutionId();
                    MatchOrders(order, oppOrder);
                }
                if (oppOrder->GetCount() == 0)
                {
                    oppOrder->SetCompleted();
                    priceQueue->pop_front();
                }
            }
        }
        return order->GetCount() == 0;
    }

    virtual void Cancel(std::shared_ptr<Order> order) override
    {
        std::unique_lock<std::mutex> l(mutex);
        while (!order->GetActivated())
            order->cv.wait(l);

        std::shared_ptr<Price> priceQueue = GetOrAssign(order->GetPrice());
        Price::iterator start;
        for (start = priceQueue->begin(); start != priceQueue->end(); start++)
            if ((*start)->GetOrderId() == order->GetOrderId())
                break;

        // No order found
        if (start == priceQueue->end())
        {
            Output::OrderDeleted(order->GetOrderId(), false, getCurrentTimestamp());
            return;
        }

        std::shared_ptr<Order> o = *start;
        if (o->GetCompleted())
        {
            Output::OrderDeleted(order->GetOrderId(), false, getCurrentTimestamp());
            return;
        }
        int cnt = o->GetCount();
        o->SetCompleted();
        priceQueue->erase(start);

        Output::OrderDeleted(order->GetOrderId(), cnt > 0, getCurrentTimestamp());
    }

    virtual void AfterExecute(std::shared_ptr<Order> order, bool filled) override
    {
        std::unique_lock<std::mutex> l(mutex);

        if (!filled)
            Output::OrderAdded(
                order->GetOrderId(),
                order->GetInstrumentId().c_str(),
                order->GetPrice(),
                order->GetCount(),
                order->GetSide() == Side::SELL,
                getCurrentTimestamp());
        // Add
        order->Activate();
    }
    virtual ~Book() = default;

private:
    std::shared_ptr<Price> GetOrAssign(price_t price)
    {
        if (!map.contains(price))
            map[price] = std::make_shared<Price>();
        return map[price];
    }

private:
    BookT<T> map;
    std::mutex mutex;
};

#endif