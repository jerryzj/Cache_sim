#include "cache.hpp"
#include "victim_cache.hpp"

VictimCache::VictimCache(const CACHE_SET &cache_setting)
    : Cache(cache_setting) {
    _cache_setting = cache_setting;
    for (auto i : _cache) {
        i.reset(); // reset cache
    }
    _Cache_Setup();
}

VictimCache::~VictimCache() = default;

bool VictimCache::_IsHit(const std::bitset<32> &addr,
                         const std::bitset<32> &victim_addr) {
    bool ret(false);
    if (Cache::_IsHit(addr)) {
        _WriteToBlock(victim_addr);
        ret = true;
    }
    return ret;
}

void VictimCache::_Insert(const std::bitset<32> &victim_addr) {
    _Replace(victim_addr);
}
