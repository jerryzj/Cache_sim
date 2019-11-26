#include "victim_cache.hpp"
#include "cache.hpp"
VictimCache::VictimCache(CACHE_SET cache_setting): _current_block(0), _current_set(0), _bit_block(0), _bit_line(0),
      _bit_tag(0), _bit_set(0), _cache_setting(cache_setting) {
    for (auto i : _cache) {
        i.reset(); // reset cache
    }
    _Cache_Setup();
}

VictimCache::~VictimCache() = default;

bool VictimCache::_IsHit(const std::bitset<32> &addr, const std::bitset<32> &victim_addr) {
    bool ret(false);
    if(_IsHit(addr)) {
        _WriteToBlock(victim_addr);
        ret = true; 
    }
    return ret;
}

void VictimCache::_Insert(const std::bitset<32> &victim_addr) {
    _Replace(victim_addr);
}






