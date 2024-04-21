#define private public
#include <iostream>
#include <assert.h>

#include "../../order_book.hpp"

bool test()
{
    OrderBook ob;
    ob.asks[9];
    ob.asks[2];
    ob.asks[3];
    ob.asks[5];
    for (auto [k, _] : ob.asks)
        std::cout << "[" << k << "]\n";
    return true;
}

int main()
{
    assert(test());
    return 1;
}