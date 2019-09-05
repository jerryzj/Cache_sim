#include "cache.hpp"

Cache::Cache(const char *config_filename) {
    // reset cache
    for (auto i : _cache) {
        i.reset();
    }
    _bit_block = 0;
    _bit_line = 0;
    _bit_set = 0;
    _bit_tag = 0;
    _current_block = 0;
    _current_set = 0;
    _cache_setting = readConfig(config_filename);
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
            _cache_setting.num_block / _cache_setting.cache_sets;
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
    _bit_tag = 32ul - _bit_block - _bit_line - _bit_set;
    assert(_bit_tag <= 29);
    for (ulint i = 0; i < _cache_setting.num_block; ++i) {
        _cache[i][31] = true;
    }
}

void Cache::run_sim(const char *trace_file) {
    std::ifstream in_file;
    char trace_line[13];

    in_file.open(trace_file, std::ios::in);
    if (in_file.fail()) {
        std::cerr << "Open trace file error" << std::endl;
        exit(-1);
    }

    while (!in_file.eof()) {
        try {
            in_file.getline(trace_line, 13);
            bool is_success = _CacheHandler(trace_line);
            if (!is_success) {
                throw std::logic_error("Cache Handler failed");
            }
        } catch (std::exception &ex) {
            in_file.close();
            std::cerr << ex.what() << std::endl;
            exit(-1);
        }
    }
    in_file.close();
    _CalHitRate();
}

void Cache::dump_result(const char *trace_file) {

    std::cout << "===================================" << std::endl;
    std::cout << "Test file: " << trace_file << std::endl;
    std::cout << "Cache size: " << _cache_setting.cache_size << "KB"
              << std::endl;
    std::cout << "Cache block size: " << _cache_setting.block_size << "B"
              << std::endl;
    switch (_cache_setting.associativity) {
    case direct_mapped:
        std::cout << "Associativity: direct_mapped" << std::endl;
        break;
    case set_associative:
        std::cout << "Associativity: " << _cache_setting.cache_sets
                  << "-way set_associative" << std::endl;
        break;
    case full_associative:
        std::cout << "Associativity: fully_associative" << std::endl;
        break;
    default:
        std::cerr << "Error associtivity setting" << std::endl;
        exit(-1);
    }
    switch (_cache_setting.replacement_policy) {
    case NONE:
        std::cout << "Replacement policy: None" << std::endl;
        break;
    case RANDOM:
        std::cout << "Replacement policy: Random" << std::endl;
        break;
    case LRU:
        std::cout << "Replacement policy: LRU" << std::endl;
        break;
    default:
        std::cerr << "Error replacement setting" << std::endl;
        exit(-1);
    }
    std::cout << "\n";
    std::cout << "Number of cache access： " << _counter.access << std::endl;
    std::cout << "Number of cache load： " << _counter.load << std::endl;
    std::cout << "Number of cache store： " << _counter.store << std::endl;
    std::cout << "Cache hit rate: " << _counter.avg_hit_rate << std::endl;
    std::cout << "===================================" << std::endl;
}

void Cache::dump_CACTI_config() {
    std::ofstream out_file("cacti.cfg");

    if (!out_file) {
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

bool Cache::_CacheHandler(char *trace_line) {
    bool is_load = false;
    bool is_store = false;
    bool is_space = false;
    bool hit = false;

    switch (trace_line[0]) {
    case 'l':
        is_load = true;
        break;
    case 's':
        is_store = true;
        break;
    case '\0':
        is_space = true;
        break;
    default:
        std::cerr << "Undefined instruction type." << std::endl;
        std::cerr << "Error line: " << trace_line << std::endl;
        return false;
    }
    auto temp = strtoul(trace_line + 2, nullptr, 16);
    std::bitset<32> addr(temp);
    hit = _IsHit(addr);

    if (hit && is_load) {
        ++_counter.access;
        ++_counter.load;
        ++_counter.load_hit;
        ++_counter.hit;
    } else if (hit && is_store) {
        ++_counter.access;
        ++_counter.store;
        ++_counter.store_hit;
        ++_counter.hit;
        // If write back, set dirty bit
        if (_cache_setting.write_policy == write_back) {
            _cache[_current_block][29] = true;
        }
    } else if ((!hit) && is_load) {
        ++_counter.access;
        ++_counter.load;
        _Read(addr);
    } else if ((!hit) && is_store) {
        ++_counter.access;
        ++_counter.store;
        _Read(addr);
        if (_cache_setting.write_policy == write_back) {
            _cache[_current_block][29] = true; // set dirty bit
        }
    } else if (is_space) {
        ++_counter.space;
    } else {
        std::cerr << "Unexpected error in _CacheHandler()" << std::endl;
        std::cerr << "ERROR line: " << trace_line << std::endl;
        return false;
    }
    return true;
}

bool Cache::_IsHit(std::bitset<32> addr) {
    bool ret = false;

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
    case direct_mapped: // nothing to do, replacement policy is not applicable
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

void Cache::_CalHitRate() {
    assert(_counter.access != 0);
    assert(_counter.load != 0);
    assert(_counter.store != 0);
    _counter.avg_hit_rate = static_cast<double>(_counter.hit) / _counter.access;
    _counter.load_hit_rate =
        static_cast<double>(_counter.load_hit) / _counter.load;
    _counter.store_hit_rate =
        static_cast<double>(_counter.store_hit) / _counter.store;
}
