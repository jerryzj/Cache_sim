#ifndef BASECACHE_HPP
#define BASECACHE_HPP

#include "datatype.hpp"
#include <cmath>
#define MAX_LINE 65536

class BaseCache {
  public:
    BaseCache();
    BaseCache(const CachePropertyStruct &);
    virtual ~BaseCache() = default;

    virtual bool Get(const addr_t &) = 0;
    virtual bool Set(const addr_t &) = 0;
    virtual bool IsHit(const addr_t &) = 0;

    CachePropertyStruct GetProperty() { return property; }

  protected:
    /*  [30]: valid [29]: dirty bit [28]~[0]: data
        [31]: enable*/
    std::bitset<32> _cache[MAX_LINE];

    // Cache properties
    CachePropertyStruct property;
};

#endif