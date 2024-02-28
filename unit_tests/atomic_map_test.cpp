#include <string>
#include <assert.h>
#include "../atomic_map.hpp"

bool test_get_single_thread()
{
  AtomicMap<std::string, int> map;
  // key not exist yet
  int res = map.Get("test");
  if (res != 0)
  {
    return false;
  }
  return true;
}

int main()
{
  assert(test_get_single_thread());
}