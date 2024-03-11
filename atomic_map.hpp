#ifndef ATOMIC_MAP_HPP
#define ATOMIC_MAP_HPP

#include <mutex>
#include <map>

template <typename T>
struct WrapperValue
{
  std::mutex lock;
  T val;
};

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

  std::map<K, V, Comparator>::iterator begin()
  {
    return map.begin();
  }

  std::map<K, V, Comparator>::iterator end()
  {
    return map.end();
  }

  size_t Erase(const K &key)
  {
    return map.erase(key);
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
  std::map<K, WrapperValue<V>, Comparator> map;
};

#endif