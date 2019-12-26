#ifndef BASECACHE_HPP
#define BASECACHE_HPP

#include "cache_setting.hpp"
#include "datatype.hpp"
#include <cmath>
#define MAX_LINE 65536

struct CachePropertyStruct {
    MAPPING_P associativity;
    REPL_P replacement_policy;
    WRITE_P write_policy;

    uint _cache_size;
    uint _block_size;
    uint _bit_offset; // # of bits of offset
    uint _bit_index;  // # of bits of index
    uint _bit_set;    // # of bits of set
    uint _bit_tag;    // # of bits of tag

    uint _num_block; // # of blocks
    uint _num_way;   // N-way
    uint _num_set;   // # of sets
};

class BaseCache {
  public:
    BaseCache();
    BaseCache(const CACHE_SET &);
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