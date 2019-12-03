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

Cache::~Cache() = default;

void Cache::_Cache_Setup() {
    assert(_cache_setting.block_size > 0);
    _cache_setting.num_block =
        (_cache_setting.cache_size << 10) / _cache_setting.block_size;
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
        std::cerr << "Invlid mapping policy" << std::endl;
        exit(-1);
    }
    _bit_tag = (32ul - _bit_block - _bit_line - _bit_set);
    assert(_bit_tag <= 29);
    for (ulint i = 0; i < _cache_setting.num_block; ++i) {
        _cache[i][31] = true;
    }
}

void Cache::dump_CACTI_config() {
    std::ofstream out_file("cacti.cfg", std::ios::out);

    // BUG: The output file doesn't work in CACTI, please fix it.
    if (out_file.fail()) {
        std::cerr << "Unable to generate cacti.cfg" << std::endl;
        exit(-1);
    }
    out_file << "# Cache size\n";
    out_file << "-size (bytes) 65536\n";
    out_file << "-Array Power Gating - \"false\" \n";
    out_file << "-WL Power Gating - \"false\"\n";
    out_file << "-CL Power Gating - \"false\"\n";
    out_file << "-Bitline floating - \"false\"\n";
    out_file << "-Interconnect Power Gating - \"false\"\n";
    out_file << "-Power Gating Performance Loss 0.01\n";
    out_file << "-block size (bytes) " << _cache_setting.block_size
             << std::endl;
    switch (_cache_setting.associativity) {
    case direct_mapped:
        out_file << "-associativity 1\n";
        break;
    case set_associative:
        out_file << "-associativity " << _cache_setting.cache_sets << std::endl;
        break;
    case full_associative:
        out_file << "-associativity 0\n";
        break;
    default:
        std::cerr << "Invalid associativity" << std::endl;
        out_file.close();
        exit(-1);
    }
    out_file << "-read-write port 1\n";
    out_file << "-exclusive read port 0\n";
    out_file << "-exclusive write port 0\n";
    out_file << "-single ended read ports 0\n";
    out_file << "-UCA bank count 1\n";
    out_file << "-technology (u) 0.090\n";
    out_file << "-Data array cell type - \"itrs-hp\"\n";
    out_file << "-Data array peripheral type - \"itrs-hp\"\n";
    out_file << "-Tag array cell type - \"itrs-hp\"\n";
    out_file << "-Tag array peripheral type - \"itrs-hp\"\n";
    out_file << "-output/input bus width 64\n";
    out_file << "-operating temperature (K) 360\n";
    out_file << "-cache type \"cache\"\n";
    out_file << "-tag size (b) \"default\"\n";
    out_file << "-access mode (normal, sequential, fast) - \"normal\"\n";
    out_file << "-design objective (weight delay, dynamic power, leakage "
                "power, cycle time, area) 0:0:0:100:0\n";
    out_file << "-deviate (delay, dynamic power, leakage power, cycle time, "
                "area) 20:100000:100000:100000:100000\n";
    out_file << "-NUCAdesign objective (weight delay, dynamic power, leakage "
                "power, cycle time, area) 100:100:0:0:100\n";
    out_file << "-NUCAdeviate (delay, dynamic power, leakage power, cycle "
                "time, area) 10:10000:10000:10000:10000\n";
    out_file << "-Optimize ED or ED^2 (ED, ED^2, NONE): \"ED^2\"\n";
    out_file << "-Cache model (NUCA, UCA)  - \"UCA\"\n";
    out_file
        << "-Wire signaling (fullswing, lowswing, default) - \"Global_30\"\n";
    out_file << "-Wire inside mat - \"semi-global\"\n";
    out_file << "-Wire outside mat - \"semi-global\"\n";
    out_file << "-Interconnect projection - \"conservative\"\n";
    out_file << "-Core count 8\n";
    out_file << "-Cache level (L2/L3) - \"L2\"\n";
    out_file << "-Add ECC - \"true\"\n";
    out_file << "-Print level (DETAILED, CONCISE) - \"DETAILED\"\n";
    out_file << "-Print input parameters - \"true\"\n";
    out_file << "-Force cache config - \"false\"\n";
    // out_file<< "-read-write port 1\n";
    // out_file<< "-read-write port 1\n";
    // out_file<< "-read-write port 1\n";
    out_file.close();
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
                                  ulint block_set) {
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
