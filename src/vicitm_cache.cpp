#include "cache.hpp"
#include "victim_cache.hpp"

VictimCache::VictimCache(const CACHE_SET &cache_setting) : Cache() {
    for (auto i : _cache) {
        i.reset(); // reset cache
    }
    _cache_setting = cache_setting;
    this->_Cache_Setup();
}
void VictimCache::_Cache_Setup() {
    std::cout << "init victim cache\n";
    // victim cache_size is in Byte!!
    assert(_cache_setting.block_size > 0);
    _cache_setting.num_block =
        (_cache_setting.cache_size) / _cache_setting.block_size;

    auto temp = _cache_setting.block_size;
    while (temp != 0u) {
        temp >>= 1;
        _bit_block++;
    }
    --_bit_block;
    std::cout << "block #" << _cache_setting.num_block << "\n";
    // Setup bit line and bit set
    switch (_cache_setting.associativity) {
    case direct_mapped:
        temp = _cache_setting.num_block;
        while (temp != 0u) {
            temp >>= 1;
            _bit_line++;
        }
        --_bit_line;
        _bit_set = 0;
        break;
    case full_associative:
        std::cout << "victim: full assoc\n";
        _bit_line = 0;
        _bit_set = 0;
        break;
    case set_associative:
        _bit_line = 0;
        assert(_cache_setting.cache_sets != 0);
        assert(_cache_setting.num_block > _cache_setting.cache_sets);
        _cache_setting.num_sets =
            (_cache_setting.num_block / _cache_setting.cache_sets);
        temp = _cache_setting.num_sets;
        while (temp != 0u) {
            temp >>= 1;
            _bit_set++;
        }
        --_bit_set;
        break;
    default:
        std::cerr << "Invlid mapping policy" << std::endl;
        exit(-1);
    }
    _bit_tag = (32ul - _bit_block - _bit_line - _bit_set);
    std::cout << _bit_tag << "::\n";
    assert(_bit_tag <= 29);
    for (ulint i = 0; i < _cache_setting.num_block; ++i) {
        _cache[i][31] = true;
    }
}
VictimCache::~VictimCache() = default;

bool VictimCache::_IsHit(const std::bitset<32> &addr,
                         const std::bitset<32> &victim_addr) {
    bool ret(false);
    for (ulint i = 0; i < _cache_setting.num_block; ++i) {
        // std::cout << i << "," << _cache[i] << "," << addr << "\n";
        if (_CheckIdent(_cache[i], addr)) {
            std::cout << "Jw";
            ret = true;
            _current_block = i;
            break;
        }
    }
    if (ret) {
        std::cout << ";";
        // if (Cache::_IsHit(addr)) {
        _WriteToBlock(victim_addr);
        //     ret = true;
    }

    return ret;
}

void VictimCache::_Insert(const std::bitset<32> &victim_addr) {
    // _Read(victim_addr);
    bool space = false;
    for (uint i = 0; i < _cache_setting.num_block; ++i) {
        if (!_cache[i][30]) {
            space = true;
            _current_block = i;
            break;
        }
    }
    if (space) {
        _WriteToBlock(victim_addr);
        if (_cache_setting.replacement_policy == LRU) {
            // need LRU hit handler
        }
    } else {
        _Replace(victim_addr);
    }
}
