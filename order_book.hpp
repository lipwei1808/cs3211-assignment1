#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <chrono>
#include <deque>
#include <map>
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
template <typename T>
using Book = std::map<price_t, std::shared_ptr<Price>, T>;

class OrderBook
{
public:
    OrderBook() = default;
    void Handle(std::shared_ptr<Order> order);
    void Cancel(std::shared_ptr<Order> order);

    std::mutex buy;
    std::mutex sell;

private:
    void Add(std::shared_ptr<Order> order);
    std::shared_ptr<Price> GetPrice(Side side, price_t price);
    bool Match(std::shared_ptr<Order> order);
    void Prepare(std::shared_ptr<Order> order);
    void Execute(std::shared_ptr<Order> order);

    Book<std::greater<price_t>> bids;
    Book<std::less<price_t>> asks;

    std::mutex order_book_lock;
    std::mutex bids_lock;
    std::mutex asks_lock;
};

#endif