#include "base_cache.hpp"

BaseCache::BaseCache(const CacheProperty &setting) : property(setting) {

    // set cache block size/bit
    property._bit_offset = log2l(setting._block_size);
    for (auto _line : _cache) {
        _line.reset();
    }
}