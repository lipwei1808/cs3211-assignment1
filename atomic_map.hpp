#ifndef ATOMIC_MAP_HPP
#define ATOMIC_MAP_HPP

#include <mutex>
#include <map>

template <typename K, typename V, typename Comparator = std::less<K>>
class AtomicMap
{
public:
  AtomicMap() = default;
  AtomicMap(const Comparator &comp) : map(comp) {}

  V &Get(K key)
  {
    if (map.find(key) != map.end())
    {
      return map[key];
    }

    return Create(key);
  }

  size_t Size() const
  {
    return map.size();
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
  std::map<K, V, Comparator> map;
};

#endif