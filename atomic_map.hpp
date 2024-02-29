#ifndef _ATOMIC_MAP_HPP_
#define _ATOMIC_MAP_HPP_

#include <mutex>
#include <unordered_map>

template <typename K, typename V>
class AtomicMap
{
public:
  V &Get(K key)
  {
    if (map.find(key) != map.end())
    {
      return map[key];
    }

    return Create(key);
  }

private:
  V &Create(K key)
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (map.find(key) == map.end())
    {
      map[key] = V();
    }

    return map[key];
  }

  std::mutex mutex;
  std::unordered_map<K, V> map;
};

#endif