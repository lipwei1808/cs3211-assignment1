#define private public
#include <iostream>
#include <assert.h>

#include "../order_book.hpp"

bool test()
{
    OrderBook ob;
    ob.asks.Get(9);
    ob.asks.Get(2);
    ob.asks.Get(3);
    ob.asks.Get(5);
    for (auto [k, _] : ob.asks.map)
        std::cout << "[" << k << "]\n";
    return true;
}

int main()
{
    assert(test());
    return 1;
}