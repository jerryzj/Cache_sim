#include "cache.hpp"

extern int simulator_verbose_output;

Cache::Cache(const CACHE_SET &cfg)
    : _current_block(0), _current_set(0), _has_evicted(false), _bit_block(0),
      _bit_line(0), _bit_tag(0), _bit_set(0) {

    for (auto i : _cache) {
        i.reset(); // reset cache
    }
    _cache_setting = cfg;
    _Cache_Setup();
}

Cache::Cache()
    : _current_block(0), _current_set(0), _has_evicted(false), _bit_block(0),
      _bit_line(0), _bit_tag(0), _bit_set(0) {}

Cache::~Cache() = default;

void Cache::_Cache_Setup() {
    assert(_cache_setting.block_size > 0);
    if (_cache_setting.type == L1) {
        _cache_setting.num_block =
            (_cache_setting.cache_size << 10) / _cache_setting.block_size;
    } else {
        _cache_setting.num_block = _cache_setting.cache_size;
    }

    auto temp = _cache_setting.block_size;
    while (temp != 0u) {
        temp >>= 1;
        _bit_block++;
    }
    --_bit_block;
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
        std::cerr << "Invalid mapping policy" << std::endl;
        exit(-1);
    }
    _bit_tag = (32ul - _bit_block - _bit_line - _bit_set);
    assert(_bit_tag <= 29);
    for (ulint i = 0; i < _cache_setting.num_block; ++i) {
        _cache[i][31] = true;
    }
}

bool Cache::IsHit() { return _has_hit; }

bool Cache::_IsHit(const std::bitset<32> &addr) {
    bool ret(false);

    switch (_cache_setting.associativity) {
    case direct_mapped:
        /*  For direct-mapped, only check if corresponding line
            is occupied. If yes, check identity.
        */
        _current_block = _GetCacheIndex(addr);
        if (_cache[_current_block][30]) {
            ret = _CheckIdent(_cache[_current_block], addr);
        } else {
            _has_space = true;
        }
        break;

    case full_associative:
        for (ulint i = 0; i < _cache_setting.num_block; ++i) {
            if (_cache[i][30]) {
                ret = _CheckIdent(_cache[i], addr);
            } else {
                _has_space = true;
            }

            if (ret) {
                _current_block = i;
                break;
            }
        }
        break;

    case set_associative:
        _current_set = _GetCacheIndex(addr);
        for (ulint i = _cache_setting.cache_sets * _current_set;
             i < ((_current_set + 1) * _cache_setting.cache_sets); ++i) {
            if (_cache[i][30]) {
                ret = _CheckIdent(_cache[i], addr);
            } else {
                _has_space = true;
            }

            if (ret) {
                _current_block = i;
                break;
            }
        }
        break;
    }

    return ret;
}

void Cache::_Read(const std::bitset<32> &addr) {
    bool space = false;
    switch (_cache_setting.associativity) {
    case direct_mapped:
        if (!_cache[_current_block][30]) {
            _WriteToBlock(addr);
        } else {
            _Replace(addr);
        }
        break;
    case full_associative:
        // Find available block
        for (uint i = 0; i < _cache_setting.num_block; ++i) {
            if (!_cache[i][30]) {
                space = true;
                _current_block = i;
                break;
            }
        }
        if (space) {
            _WriteToBlock(addr);
            if (_cache_setting.replacement_policy == LRU) {
                _LRUHitHandle();
            }
        } else {
            _Replace(addr);
        }
        break;
    case set_associative:
        space = false;
        for (ulint i = (_current_set * _cache_setting.cache_sets);
             i < ((_current_set + 1)) * _cache_setting.cache_sets; i++) {
            if (!_cache[i][30]) {
                space = true;
                _current_block = i;
                break;
            }
        }
        if (space) {
            _WriteToBlock(addr);
            if (_cache_setting.replacement_policy == LRU) {
                _LRUHitHandle();
            }
        } else {
            _Replace(addr);
        }
        break;
    }
}

void Cache::_Replace(const std::bitset<32> &addr) {
    // Find victim block
    switch (_cache_setting.associativity) {
    case direct_mapped:
        // nothing to do, replacement policy is not applicable
        // on direct mapped cache
        break;
    case full_associative:
        if (_cache_setting.replacement_policy == RANDOM) {
            GetBlockByRandom();
        } else if (_cache_setting.replacement_policy == LRU) {
            GetBlockByLRU();
        }
        break;
    case set_associative:
        if (_cache_setting.replacement_policy == RANDOM) {
            GetBlockByRandom();
        } else if (_cache_setting.replacement_policy == LRU) {
            GetBlockByLRU();
        }
        break;
    }
    // If the victim block is dirty, write back to RAM
    if (_cache[_current_block][29]) {
        _Drop();
    }
    // Write new data from RAM to Cache
    _WriteToBlock(addr);
}

void Cache::_Drop() {
    // Set dirty bit and hit bit to flase
    _cache[_current_block][29] = false;
    _cache[_current_block][30] = false;
}

std::bitset<32> Cache::_CvtToAddr(const ulint block_set) {
    std::bitset<32> addr;
    addr.reset();
    std::bitset<32> index(block_set);

    auto fill_tag_bit = [_c = this](std::bitset<32> &dst_addr,
                                    const ulint block_set) {
        for (uint i = 31, j = 28; i > (31ul - _c->_bit_tag); --i, --j) {
            dst_addr[i] = _c->_cache[block_set][j];
        }
    };

    switch (_cache_setting.associativity) {
    case direct_mapped:
        for (ulint i = (_bit_block), j = 0; i < (_bit_block + _bit_line);
             ++i, ++j) {
            addr[i] = index[j];
        }
        fill_tag_bit(addr, block_set);

        break;
    case full_associative:
        fill_tag_bit(addr, block_set);

        break;
    case set_associative:
        for (ulint i = (_bit_block), j = 0; i < (_bit_block + _bit_set);
             ++i, ++j) {
            addr[i] = index[j];
        }
        fill_tag_bit(addr, block_set);

        break;
    }

    return addr;
}

void Cache::Ready(const std::bitset<32> &addr) {
    _current_addr = addr;
    _current_block = 0;
    _current_set = 0;
    _has_space = false;

    _has_hit = _IsHit(_current_addr);

    if (!_has_hit) {
        switch (_cache_setting.associativity) {
        case direct_mapped:
            if (!_IfBlockAvailable()) {
                _has_evicted = true;
            }
            break;
        case full_associative:
        case set_associative:
            if (!_IfBlockAvailable()) {
                _has_evicted = true;
                if (_cache_setting.replacement_policy == RANDOM) {
                    GetBlockByRandom();
                } else if (_cache_setting.replacement_policy == LRU) {
                    GetBlockByLRU();
                }
            }
            break;
        }
    }
}

void Cache::_Update() {
    if (_has_evicted) {
        _WriteToBlock(_current_addr);
    } else {
        _Read(_current_addr);
    }
}

void Cache::_WriteToBlock(const std::bitset<32> &addr) {
    for (uint i = 31, j = 28; i > (31ul - _bit_tag); --i, --j) {
        _cache[_current_block][j] = addr[i];
        assert(j >= 0);
    }
    _cache[_current_block][30] = true;
}

ulint Cache::_GetCacheIndex(const std::bitset<32> &addr) {
    std::bitset<32> temp_cache_line;
    temp_cache_line.reset();
    if (_cache_setting.associativity == set_associative) {
        for (ulint i = (_bit_block), j = 0; i < (_bit_block + _bit_set);
             ++i, ++j) {
            temp_cache_line[j] = addr[i];
        }
    } else {
        for (ulint i = (_bit_block), j = 0; i < (_bit_block + _bit_line);
             ++i, ++j) {
            temp_cache_line[j] = addr[i];
        }
    }
    return temp_cache_line.to_ulong();
}

bool Cache::_CheckIdent(const std::bitset<32> &cache,
                        const std::bitset<32> &addr) {
    for (uint i = 31, j = 28; i > (31ul - _bit_tag); --i, --j) {
        if (addr[i] != cache[j]) {
            return false;
        }
    }
    return true;
}

void Cache::GetBlockByRandom() {
    std::random_device rd;
    std::mt19937_64 generator(rd());
    std::uniform_int_distribution<int> unif(0, INT32_MAX);

    switch (_cache_setting.associativity) {
    case direct_mapped:
        break;
    case full_associative:
        _current_block = static_cast<ulint>(
            unif(generator) / (INT32_MAX / _cache_setting.num_block + 1));

        break;

    case set_associative:
        ulint temp = static_cast<ulint>(
            unif(generator) / (INT32_MAX / _cache_setting.cache_sets + 1));
        _current_block = _current_set * _cache_setting.cache_sets + temp;

        break;
    }
}

bool Cache::_IfBlockAvailable() { return _has_space; }

void Cache::_LRUHitHandle() {}

ulint Cache::GetBlockByLRU() {
    // TODO: Your part 1 assignment
    ulint res(0);

    return res;
}
