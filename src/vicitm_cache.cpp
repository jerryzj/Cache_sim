#include "cache.hpp"
#include "victim_cache.hpp"

VictimCache::VictimCache(const CACHE_SET &cache_setting) : Cache() {
    for (auto i : _cache) {
        i.reset(); // reset cache
    }
    assert(cache_setting.associativity == full_associative);
    _cache_setting = cache_setting;
    this->_Cache_Setup();
}

VictimCache::~VictimCache() = default;

bool VictimCache::_IsHit(const std::bitset<32> &addr,
                         const std::bitset<32> &victim_addr) {
    bool ret(false);
    for (ulint i = 0; i < _cache_setting.num_block; ++i) {
        if (_CheckIdent(_cache[i], addr)) {
            ret = true;
            _current_block = i;
            break;
        }
    }
    if (ret) {
        _WriteToBlock(victim_addr);
    }

    return ret;
}

void VictimCache::_Insert(const std::bitset<32> &victim_addr) {
    _Read(victim_addr);
}
