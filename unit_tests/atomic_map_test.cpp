#define private public
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <assert.h>

#include "../order_book.hpp"
#include "../order.hpp"
#include "../atomic_map.hpp"

struct test
{
  int x1;
};

bool test_get_single_thread()
{
  std::cout << "\nStarting [test_get_single_thread]\n";
  AtomicMap<std::string, int> map;
  // key not exist yet
  int res = map.Get("test");
  if (res != 0)
  {
    return false;
  }
  std::cout << "Ending [test_get_single_thread]\n\n";
  return true;
}

bool test_return_reference()
{
  std::cout << "\nStarting [test_return_reference]\n";
  AtomicMap<std::string, test> map;
  test &t1 = map.Get("test");
  t1.x1 = 999;

  test t2 = map.Get("test");
  if (&t1 == &t2)
  {
    return false;
  }
  test &t3 = map.Get("test");
  std::cout << "Ending [test_return_reference]\n\n";
  return t1.x1 == t3.x1 && &t1 == &t3;
}

bool test_get_multiple_thread()
{
  std::cout << "\nStarting [test_get_multiple_thread]\n";
  AtomicMap<std::string, test> map;
  test *results[10];
  std::mutex lock;
  auto runner = [&](int i)
  {
    test &t1 = map.Get("test");
    std::this_thread::sleep_for(std::chrono::milliseconds(2 * 1000));
    std::unique_lock<std::mutex> ul(lock);
    results[i] = &t1;
  };
  std::thread c[10];
  for (int i = 0; i < 10; i++)
  {
    c[i] = std::thread(runner, i);
  }
  test *add = nullptr;
  for (int i = 0; i < 10; i++)
  {
    c[i].join();
    if (add == nullptr)
    {
      if (results[i] == nullptr)
      {
        return false;
      }
      add = results[i];
    }
    else if (results[i] != add)
    {
      return false;
    }
  }
  std::cout << "Ending [test_get_multiple_thread]\n\n";
  return true;
}

bool test_sort_atomic_map_no_sorter()
{
  std::cout << "\nStarting [test_sort_atomic_map_no_sorter]\n";
  AtomicMap<int, char> map;
  map.Get(8);
  map.Get(2);
  map.Get(5);
  map.Get(9);
  map.Get(1);
  int last = -1;
  for (auto [_, v] : map.map)
  {
    if (last > v)
      return false;
    else
      last = v;
  }
  std::cout << "Ending [test_sort_atomic_map_no_sorter]\n\n";
  return true;
}

bool test_sort_atomic_map_greater_sort()
{
  std::cout << "\nStarting [test_sort_atomic_map_greater_sort]\n";
  AtomicMap<int, char, std::greater<int>> map;
  map.Get(8);
  map.Get(2);
  map.Get(5);
  map.Get(9);
  map.Get(1);
  int last = -1;
  for (auto [_, v] : map.map)
  {
    {
      if (last == -1 || last >= v)
        last = v;
      else
        return false;
    }
  }
  std::cout << "Ending [test_sort_atomic_map_greater_sort]\n\n";
  return true;
}

bool test_sort_atomic_map_custom_sort()
{
  std::cout << "\nStarting [test_sort_atomic_map_greater_sort]\n";
  AtomicMap<int, char, PriceComparator> map(PriceComparator{Side::BUY});
  // map.Get(8);
  // map.Get(2);
  // map.Get(5);
  // map.Get(9);
  // map.Get(1);
  // int last = -1;
  // for (auto [_, v] : map.map)
  // {
  //   {
  //     if (last == -1 || last >= v)
  //       last = v;
  //     else
  //       return false;
  //   }
  // }
  // std::cout << "Ending [test_sort_atomic_map_greater_sort]\n\n";
  return true;
}

int main()
{
  std::cout << "Starting unit test\n";
  assert(test_get_single_thread());
  assert(test_return_reference());
  assert(test_get_multiple_thread());
  assert(test_sort_atomic_map_no_sorter());
  assert(test_sort_atomic_map_greater_sort());
  std::cout << "Success\n";
}