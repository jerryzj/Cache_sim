#ifndef _BASE_CACHE_HPP
#define _BASE_CACHE_HPP

#include "datatype.hpp"
#include <cmath>

const int MAX_LINE = 65536;

class BaseCache {
  public:
    BaseCache();
    BaseCache(const CacheProperty &);
    virtual ~BaseCache() = default;

    virtual bool Get(const addr_t &) = 0;
    virtual bool Set(const addr_t &) = 0;
    virtual bool IsHit(const addr_t &) = 0;

    CacheProperty GetProperty() { return property; }

  protected:
    /*  [47]: valid [46]: dirty bit [45]~[0]: data */
    addr_t _cache[MAX_LINE];

    // Cache properties
    CacheProperty property;
};

#endif