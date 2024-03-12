#ifndef ATOMIC_MAP_HPP
#define ATOMIC_MAP_HPP

#include <mutex>
#include <map>

template <typename T>
struct WrapperValue
{
  std::mutex lock;
  bool initialised;
  T val;
  WrapperValue() = default;
  WrapperValue(const WrapperValue &v) : initialised(v.initialised), val(v.val) {}
  T &Get()
  {
    std::unique_lock<std::mutex> l(lock);
    return val;
  }
};

template <typename K, typename V, typename Comparator = std::less<K>>
class AtomicMap
{
public:
  AtomicMap() = default;
  AtomicMap(const Comparator &comp) : map(comp) {}

  WrapperValue<V> &Get(K key)
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

  std::map<K, WrapperValue<V>, Comparator>::iterator begin()
  {
    return map.begin();
  }

  std::map<K, WrapperValue<V>, Comparator>::iterator end()
  {
    return map.end();
  }

  size_t Erase(const K &key)
  {
    return map.erase(key);
  }

private:
  WrapperValue<V> &Create(K key)
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (map.find(key) == map.end())
    {
      WrapperValue<V> v;
      map.emplace(key, v);
    }

    return map[key];
  }

  std::mutex mutex;
  std::map<K, WrapperValue<V>, Comparator> map;
};

#endif