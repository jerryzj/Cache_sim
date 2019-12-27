#include "base_cache.hpp"

BaseCache::BaseCache(const CachePropertyStruct &setting) : property(setting) {

    // set cache block size/bit
    property._bit_offset = log2l(setting._block_size - 1);
}