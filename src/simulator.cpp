#include "simulator.hpp"

extern int simulator_verbose_output;

Simulator::Simulator(const std::string &cache_cfg,
                     const std::string &program_trace)
    : cache_cfg_file(cache_cfg), trace_file(program_trace), _has_victim(false) {
    ReadConfig();
    CacheSetup();
}

Simulator::~Simulator() = default;

void Simulator::ReadConfig() {
    // TODO:
    // it's time to integrate other well-supported config format, i.e json, yaml
    // it's very un-maintainable to manipulate with plain text config file.
    if (simulator_verbose_output) {
        std::cout << "Reading cache config file" << std::endl;
    }

    CACHE_SET _cache_conf;
    _cache_conf.type = 0; // main cache
    bool is_directedmap_flag(false);
    std::list<std::string> config_raw, config_ready;

    // Preprocessing config file
    config_raw = readFile(cache_cfg_file.c_str());
    config_ready = removeComments(config_raw);

    // Assert config lines is in the right range
    assert(config_ready.size() >= 3 && config_ready.size() <= 9);

    // Read cache size
    assert(readParameter(config_ready.front(), _cache_conf.cache_size));
    config_ready.pop_front();

    // Read cache line size
    assert(readParameter(config_ready.front(), _cache_conf.block_size));
    config_ready.pop_front();

    // Read cache associativity
    switch (stoi(config_ready.front())) {
    case 1:
        _cache_conf.associativity = direct_mapped;
        break;
    case 2:
        _cache_conf.associativity = set_associative;
        break;
    case 3:
        _cache_conf.associativity = full_associative;
        break;
    default:
        std::cerr << "Invalid Associativity" << std::endl;
        std::exit(-1);
    }
    config_ready.pop_front();

    if (_cache_conf.associativity == direct_mapped) {
        // Note that replacement policy is invalid for direct-mapped cache
        _cache_conf.replacement_policy = NONE;
        is_directedmap_flag = true;
    } else if (_cache_conf.associativity == set_associative) {
        // Read set size
        assert(readParameter(config_ready.front(), _cache_conf.cache_sets));
        config_ready.pop_front();
    }
    if (!is_directedmap_flag) {
        // Read replacement policy
        switch (stoi(config_ready.front())) {
        case 1:
            _cache_conf.replacement_policy = RANDOM;
            break;
        case 2:
            _cache_conf.replacement_policy = LRU;
            break;
        // Add your own policy here, ex: RRIP
        default:
            std::cerr << "Invalid replacement policy" << std::endl;
            std::exit(-1);
        }
        config_ready.pop_front();
    }
    this->_cache_setting = _cache_conf;

    if (simulator_verbose_output) {
        std::cout << "Main cache parameters have been setup." << std::endl;
    }

    while (!config_ready.empty() && config_ready.front() != "---")
        config_ready.pop_front();
    if (config_ready.empty()) {
        return;
    }
    config_ready.pop_front();
    this->_has_victim = true;
    _cache_conf.type = 1;
    _cache_conf.cache_size = stoi(config_ready.front());
    _cache_conf.associativity = full_associative;
    this->_victim_setting = _cache_conf;

    if (simulator_verbose_output) {
        std::cout << "Victim cache parameters have been setup." << std::endl;
    }

    return;
}

void Simulator::CacheSetup() {
    this->main_cache = new Cache(this->_cache_setting);
    if (_has_victim) {
        this->victim_cache = new VictimCache(this->_victim_setting);
    }
}

void Simulator::RunSimulation() {
    std::ifstream in_file((this->trace_file).c_str(), std::ios::in);
    char trace_line[13];

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

bool Simulator::_CacheHandler(char *trace_line) {
    bool is_load(false), is_store(false), is_space(false);

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

    std::bitset<32> poten_victim = this->main_cache->_Evicted(addr);
    if (is_load) {
        ++_counter.access;
        ++_counter.load;
        if (simulator_verbose_output) {
            std::cout << "Load " << temp;
        }

        if (this->main_cache->CheckIfHit(addr)) {
            ++_counter.load_hit;
            ++_counter.hit_in_main;
            if (simulator_verbose_output) {
                std::cout << ", hit in main cache!";
            }
        } else if (this->_has_victim) {
            assert(this->victim_cache != nullptr);
            if (this->main_cache->_has_evicted &&
                this->victim_cache->_IsHit(addr, poten_victim)) {
                ++_counter.load_hit;
                ++_counter.hit_in_victim;
                if (simulator_verbose_output) {
                    std::cout << ", hit in victim cache!";
                }
                this->main_cache->_Update();
            } else {
                if (simulator_verbose_output) {
                    std::cout << ", miss";
                }
                this->main_cache->_Update();
                this->victim_cache->_Insert(poten_victim);
            }
        } else {
            if (simulator_verbose_output) {
                std::cout << ", miss";
            }
            this->main_cache->_Read(addr);
        }
    } else if (is_store) {
        ++_counter.access;
        ++_counter.store;
        if (simulator_verbose_output) {
            std::cout << "Store " << temp;
        }

        if (this->main_cache->CheckIfHit(addr)) {
            ++_counter.store_hit;
            ++_counter.hit_in_main;
            if (simulator_verbose_output) {
                std::cout << ", hit in main cache!";
            }
        } else if (this->_has_victim) {
            assert(this->victim_cache != nullptr);
            if (this->victim_cache->_IsHit(addr, poten_victim)) {
                ++_counter.store_hit;
                ++_counter.hit_in_victim;
                if (simulator_verbose_output) {
                    std::cout << ", hit in victim cache!";
                }
                this->main_cache->_Update();
            } else {
                if (simulator_verbose_output) {
                    std::cout << ", miss";
                }
                this->main_cache->_Update();
                this->victim_cache->_Insert(poten_victim);
            }
        } else {
            if (simulator_verbose_output) {
                std::cout << ", miss";
            }
            this->main_cache->_Read(addr);
        }
    } else if (is_space) {
        ++_counter.space;
    } else {
        std::cerr << "Unexpected error in _CacheHandler()" << std::endl;
        std::cerr << "ERROR line: " << trace_line << std::endl;
        return false;
    }
    if (simulator_verbose_output) {
        std::cout << std::endl;
    }
    return true;
}

void Simulator::_CalHitRate() {
    assert(_counter.access != 0);
    assert(_counter.load != 0);
    assert(_counter.store != 0);
    _counter.hit = _counter.hit_in_main + _counter.hit_in_victim;
    _counter.avg_hit_rate = static_cast<double>(_counter.hit) / _counter.access;
    _counter.load_hit_rate =
        static_cast<double>(_counter.load_hit) / _counter.load;
    _counter.store_hit_rate =
        static_cast<double>(_counter.store_hit) / _counter.store;
}

void Simulator::DumpResult() {

    // TODO: dump simulation results to yaml file,
    //       then add another yaml parser to verify correctness.
    std::cout << "Test file: " << this->trace_file << std::endl;

    std::cout << "===================================" << std::endl;
    std::cout << "┌----------------┐" << std::endl;
    std::cout << "│  Main Cache    │" << std::endl;
    std::cout << "└----------------┘" << std::endl;
    _ShowSettingInfo(this->_cache_setting);
    if (this->_has_victim) {
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "┌----------------┐" << std::endl;
        std::cout << "│  Victim Cache  │" << std::endl;
        std::cout << "└----------------┘" << std::endl;
        _ShowSettingInfo(this->_victim_setting);
    }
    std::cout << "===================================" << std::endl;

    std::cout << "Number of cache access: " << _counter.access << std::endl;
    std::cout << "Number of cache load: " << _counter.load << std::endl;
    std::cout << "Number of cache store: " << _counter.store << std::endl;
    std::cout << "Number of total cache hit: " << _counter.hit << std::endl;
    if (this->_has_victim) {
        std::cout << " - Number of hit in main cache: " << _counter.hit_in_main
                  << std::endl;
        std::cout << " - Number of hit in victim cache: "
                  << _counter.hit_in_victim << std::endl;
    }

    std::cout << "Cache hit rate: " << std::setprecision(6)
              << _counter.avg_hit_rate << std::endl;
}

void Simulator::DumpCACTIConfig() {
    _DumpCACTIConfig("cacti_main.cfg", this->_cache_setting);
    if (_has_victim)
        _DumpCACTIConfig("cacti_victim.cfg", this->_victim_setting);
}

void Simulator::_ShowSettingInfo(const CACHE_SET &_cache_setting) {
    if (_cache_setting.type == 1)
        std::cout << "Cache size: " << _cache_setting.cache_size << " blocks"
                  << std::endl;
    else
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
}

void Simulator::_DumpCACTIConfig(const std::string filename,
                                 const CACHE_SET &_cache_setting) {
    std::ofstream out_file(filename, std::ios::out);

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