#define private public
#include <string>
#include <thread>
#include <memory>
#include <mutex>
#include <vector>
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
  WrapperValue<int> res = map.Get("test");
  if (res.val != 0)
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
  test &t1 = map.Get("test").Get();
  t1.x1 = 999;

  test t2 = map.Get("test").Get();
  if (&t1 == &t2)
  {
    return false;
  }
  test &t3 = map.Get("test").Get();
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
    test &t1 = map.Get("test").Get();
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
    if (last > v.Get())
      return false;
    else
      last = v.Get();
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
      if (last == -1 || last >= v.Get())
        last = v.Get();
      else
        return false;
    }
  }
  std::cout << "Ending [test_sort_atomic_map_greater_sort]\n\n";
  return true;
}

bool test_iterators()
{
  std::cout << "\nStarting [test_iterators]\n";
  AtomicMap<int, char> map;
  map.Get(5);
  map.Get(2);
  map.Get(3);
  map.Get(1);
  std::vector<int> answers({1, 2, 3, 5});
  int i = 0;
  for (auto s = map.begin(); s != map.end(); s++)
  {
    if (s->first != answers[i])
    {
      return false;
    }
    i++;
  }
  std::cout << "Ending [test_iterators]\n\n";
  return true;
}

bool test_pointer_values()
{
  std::cout << "\nStarting [test_pointer_values]\n";
  AtomicMap<int, std::shared_ptr<char>> map;
  WrapperValue<std::shared_ptr<char>> w = map.Get(1);
  if (w.initialised)
  {
    return false;
  }

  if (w.val != nullptr)
  {
    return false;
  }

  {
    std::scoped_lock<std::mutex> l(w.lock);
    w.initialised = true;
    w.val = std::make_shared<char>('c');
  }
  if (!w.initialised)
  {
    return false;
  }

  if (*w.val != 'c')
  {
    return false;
  }

  std::cout << "Ending [test_pointer_values]\n\n";
  return true;
}

bool test_multithread_pointer_initialisation()
{
  std::cout << "\nStarting [test_multithread_pointer_initialisation]\n";
  int numThreads = 10;
  AtomicMap<int, std::shared_ptr<int>> map;
  std::mutex mutex;
  int counter = 0;
  std::thread threads[numThreads];
  auto runner = [&](int i)
  {
    WrapperValue<std::shared_ptr<int>> &w = map.Get(1);
    {
      std::scoped_lock sl(w.lock);
      if (!w.initialised)
      {
        w.val = std::make_shared<int>(i);
        w.initialised = true;
      }
    }

    std::unique_lock l(mutex);
    counter++;
  };
  for (int i = 0; i < numThreads; i++)
  {
    threads[i] = std::thread(runner, i);
  }

  for (int i = 0; i < numThreads; i++)
  {
    threads[i].join();
  }

  if (counter != numThreads)
  {
    std::cout << "C\n";
    return false;
  }
  WrapperValue<std::shared_ptr<int>> &w = map.Get(1);
  if (!w.initialised) {
    std::cout << "D\n";
    return false;
  }

  std::cout << *w.val << std::endl;

  std::cout << "Ending [test_multithread_pointer_initialisation]\n\n";
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
  assert(test_iterators());
  assert(test_pointer_values());
  assert(test_multithread_pointer_initialisation());
  std::cout << "Success\n";
}