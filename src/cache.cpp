#include "cache.hpp"

extern int simulator_verbose_output;

Cache::Cache(const CACHE_SET &cfg)
    : _current_block(0), _current_set(0), _bit_block(0), _bit_line(0),
      _bit_tag(0), _bit_set(0) {

    for (auto i : _cache) {
        i.reset(); // reset cache
    }
    _cache_setting = cfg;
    _Cache_Setup();
}
Cache::Cache()
    : _current_block(0), _current_set(0), _bit_block(0), _bit_line(0),
      _bit_tag(0), _bit_set(0) {}
Cache::~Cache() = default;

void Cache::_Cache_Setup() {
    assert(_cache_setting.block_size > 0);
    if (_cache_setting.type == 0)
        _cache_setting.num_block =
            (_cache_setting.cache_size << 10) / _cache_setting.block_size;
    else
        _cache_setting.num_block = _cache_setting.cache_size;

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

bool Cache::CheckIfHit(const std::bitset<32> &addr) {
    return this->_IsHit(addr);
}

bool Cache::_IsHit(const std::bitset<32> &addr) {
    bool ret(false);

    if (_cache_setting.associativity == direct_mapped) {
        _current_block = _GetCacheIndex(addr);
        assert(_cache[_current_block][31] == true);
        if (_cache[_current_block][30]) {
            ret = _CheckIdent(_cache[_current_block], addr);
        }
    } else if (_cache_setting.associativity == full_associative) {
        for (ulint i = 0; i < _cache_setting.num_block; ++i) {
            if (_cache[i][30]) {
                ret = _CheckIdent(_cache[i], addr);
            }
            if (ret) {
                _current_block = i;
                break;
            }
        }
    } else { // Set associative
        _current_set = _GetCacheIndex(addr);
        for (ulint i = _cache_setting.cache_sets * _current_set;
             i < ((_current_set + 1) * _cache_setting.cache_sets); ++i) {
            if (_cache[i][30]) {
                ret = _CheckIdent(_cache[i], addr);
            }
            if (ret) {
                _current_block = i;
                break;
            }
        }
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
        space = false;
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
                // need LRU hit handler
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
                // need LRU hit handler
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
            std::random_device rd;
            std::mt19937_64 generator(rd());
            std::uniform_int_distribution<int> unif(0, INT32_MAX);
            _current_block = static_cast<ulint>(
                unif(generator) / (INT32_MAX / _cache_setting.num_block + 1));
        } else if (_cache_setting.replacement_policy == LRU) {
            // Do sth.
        }
        break;
    case set_associative:
        if (_cache_setting.replacement_policy == RANDOM) {
            std::random_device rd;
            std::mt19937_64 generator(rd());
            std::uniform_int_distribution<int> unif(0, INT32_MAX);
            ulint temp = static_cast<ulint>(
                unif(generator) / (INT32_MAX / _cache_setting.cache_sets + 1));
            _current_block = _current_set * _cache_setting.cache_sets + temp;
        } else if (_cache_setting.replacement_policy == LRU) {
            // Do sth.
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

std::bitset<32> Cache::_CvtToAddr(const std::bitset<32> &cache_line,
                                  const ulint block_set) {
    std::bitset<32> addr;
    addr.reset();
    std::bitset<32> index(block_set);

    switch (_cache_setting.associativity) {
    case direct_mapped:
        for (ulint i = (_bit_block), j = 0; i < (_bit_block + _bit_line);
             ++i, ++j) {
            addr[i] = index[0];
        }

        for (uint i = 31, j = 28; i > (31ul - _bit_tag); --i, --j) {
            addr[i] = cache_line[j];
        }

        break;
    case full_associative:
        for (uint i = 31, j = 28; i > (31ul - _bit_tag); --i, --j) {
            addr[i] = cache_line[j];
        }
        break;
    case set_associative:
        for (ulint i = (_bit_block), j = 0; i < (_bit_block + _bit_set);
             ++i, ++j) {
            addr[i] = index[j];
        }

        for (uint i = 31, j = 28; i > (31ul - _bit_tag); --i, --j) {
            addr[i] = cache_line[j];
        }
        break;
    }

    return addr;
}

std::bitset<32> Cache::_Evicted(const std::bitset<32> &addr) {
    _cur_addr = addr;
    _has_evicted = false;
    bool space(false);

    if (_IsHit(addr)) {
        _poten_victim |= std::bitset<32>(0xffffffff);
    } else {
        switch (_cache_setting.associativity) {
        case direct_mapped:
            _current_block = _GetCacheIndex(addr);
            if (_cache[_current_block][30] &&
                !_CheckIdent(_cache[_current_block], addr)) {
                _has_evicted = true;
                _poten_victim =
                    this->_CvtToAddr(_cache[_current_block], _current_block);
            }
            break;
        case full_associative:
            for (uint i = 0; i < _cache_setting.num_block; ++i) {
                if (!_cache[i][30]) {
                    space = true;
                    _poten_victim |= std::bitset<32>(0xffffffff);
                    break;
                }
            }
            if (space) {
                break;
            }
            _has_evicted = true;
            if (_cache_setting.replacement_policy == RANDOM) {
                std::random_device rd;
                std::mt19937_64 generator(rd());
                std::uniform_int_distribution<int> unif(0, INT32_MAX);
                do {
                    _current_block = static_cast<ulint>(
                        unif(generator) /
                        (INT32_MAX / _cache_setting.num_block + 1));
                } while (_CheckIdent(_cache[_current_block], addr));
                _poten_victim =
                    this->_CvtToAddr(_cache[_current_block], _current_block);

            } else if (_cache_setting.replacement_policy == LRU) {
                // Do sth.
            }
            break;
        case set_associative:
            _current_set = _GetCacheIndex(addr);

            for (ulint i = (_current_set * _cache_setting.cache_sets);
                 i < ((_current_set + 1)) * _cache_setting.cache_sets; i++) {
                if (!_cache[i][30]) {
                    space = true;
                    _poten_victim |= std::bitset<32>(0xffffffff);
                    break;
                }
            }
            if (space) {
                break;
            }
            _has_evicted = true;
            if (_cache_setting.replacement_policy == RANDOM) {
                std::random_device rd;
                std::mt19937_64 generator(rd());
                std::uniform_int_distribution<int> unif(0, INT32_MAX);
                do {
                    ulint temp = static_cast<ulint>(
                        unif(generator) /
                        (INT32_MAX / _cache_setting.cache_sets + 1));
                    _current_block =
                        _current_set * _cache_setting.cache_sets + temp;
                } while (_CheckIdent(_cache[_current_block], addr));
                _poten_victim =
                    this->_CvtToAddr(_cache[_current_block], _current_set);
            } else if (_cache_setting.replacement_policy == LRU) {
                // Do sth.
            }
            break;
        }
    }

    return _poten_victim;
}

void Cache::_Update() {
    if (_has_evicted) {
        _WriteToBlock(_cur_addr);
    } else {
        _Read(_cur_addr);
    }
}

void Cache::_WriteToBlock(const std::bitset<32> &addr) {
    for (uint i = 31, j = 28; i > (31ul - _bit_tag); --i, --j) {
        _cache[_current_block][j] = addr[i];
        assert(j > 0);
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
