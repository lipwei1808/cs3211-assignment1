#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <memory>
#include <mutex>
#include <assert.h>

#include "atomic_map.hpp"
#include "book.hpp"
#include "order.hpp"

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
    void Prepare(std::shared_ptr<Order> order);
    void Execute(std::shared_ptr<Order> order);
    BaseBook * GetBook(Side side);
    BaseBook * GetOtherBook(Side side);

    Book<std::greater<price_t>> bids;
    Book<std::less<price_t>> asks;

    std::mutex order_book_lock;
};

#endif