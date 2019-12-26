#include "base_cache.hpp"
#include <random>

class MainCache : public BaseCache {
  public:
    MainCache(const CACHE_SET &);
    ~MainCache();
    bool Get(const addr_t &);
    bool Set(const addr_t &);
    bool IsHit(const addr_t &);

  protected:
    void _HitHandle(const addr_t &);
    void _Replace(const addr_t &);
    ulint _GetCacheBlockIndex(const addr_t &addr);
    ulint _GetIndexByLRU(const addr_t &addr);
    ulint _GetIndexByRandom(const addr_t &addr);
    ulint _GetSetNumber(const addr_t &addr);
};
