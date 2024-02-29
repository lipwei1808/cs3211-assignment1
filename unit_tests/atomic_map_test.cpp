#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <assert.h>
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

int main()
{
  std::cout << "Starting unit test\n";
  assert(test_get_single_thread());
  assert(test_return_reference());
  assert(test_get_multiple_thread());
  std::cout << "Success\n";
}