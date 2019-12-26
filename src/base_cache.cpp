#include "base_cache.hpp"

BaseCache::BaseCache(const CACHE_SET &setting) {
    property.replacement_policy = setting.replacement_policy;
    property._cache_size = setting.cache_size;
    property._block_size = setting.block_size;
    // set cache block size/bit
    property._bit_offset = log2l(setting.block_size - 1);
}