#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <assert.h>

#include "atomic_map.hpp"
#include "order.hpp"

inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

typedef std::deque<std::shared_ptr<Order>> Price;

// TODO: Check if bids_lock/asks_lock required when using AtomicMap
class OrderBook
{
public:
    OrderBook() = default;
    template <Side side>
    void Handle(std::shared_ptr<Order> order);
    template <Side side>
    void Cancel(std::shared_ptr<Order> order)
    {
        assert(order->GetSide() == side);
        std::unique_lock<std::mutex> l;
        if constexpr (side == Side::BUY)
            l = std::unique_lock<std::mutex>(bids_lock);
        else
            l = std::unique_lock<std::mutex>(asks_lock);

        std::shared_ptr<Price> priceLevel = GetPrice<side>(order->GetPrice());
        bool found = false;
        for (auto start = priceLevel->begin(); start != priceLevel->end(); start++)
            if (order->GetOrderId() == (*start)->GetOrderId())
            {
                found = true;
                priceLevel->erase(start);
            }
        if (priceLevel->size() == 0)
        {
            // Remove from map of prices
            size_t num = ([&]() -> size_t {
                if constexpr (side == Side::BUY) 
                    return bids.Erase(order->GetPrice());
                else 
                    return asks.Erase(order->GetPrice()); 
            })();
            assert(num == 1);
        }
        Output::OrderDeleted(order->GetOrderId(), found, getCurrentTimestamp());
    }

private:
    template <Side side>
    void Add(std::shared_ptr<Order> order);
    template <Side side>
    std::shared_ptr<Price> GetPrice(price_t price);
    template <Side side>
    bool Execute(std::shared_ptr<Order> order);
    void MatchOrders(std::shared_ptr<Order> o1, std::shared_ptr<Order> o2);

    AtomicMap<price_t, WrapperValue<std::shared_ptr<Price>>, std::greater<price_t>> bids;
    AtomicMap<price_t, WrapperValue<std::shared_ptr<Price>>> asks;

    std::mutex order_book_lock;
    std::mutex bids_lock;
    std::mutex asks_lock;
};

#endif