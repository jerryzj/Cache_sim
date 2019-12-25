#include "simulator.hpp"

Simulator::Simulator(const std::string &cache_cfg,
                     const std::string &program_trace)
    : cache_cfg_file(cache_cfg), trace_file(program_trace), _has_victim(false) {
    ReadConfig();
    main_cache = std::make_unique<Cache>(this->_cache_list[0]);
    inst_loader = std::make_unique<InstructionLoader>(trace_file);
}

Simulator::~Simulator() = default;

void Simulator::ReadConfig() {
    std::vector<CACHE_SET> caches;
    ParseCacheConfig(cache_cfg_file.c_str(), this->_cache_list);

    return;
}

void Simulator::RunSimulation() {
    while (inst_loader->IfAvailable()) {
        try {
            bool is_success = _CacheHandler(inst_loader->GetNextInst());
            if (!is_success) {
                throw std::logic_error("Cache Handler failed");
            }
        } catch (std::exception &ex) {
            std::cerr << ex.what() << std::endl;
            exit(-1);
        }
    }
    _CalHitRate();
}

bool Simulator::_CacheHandler(inst_t inst) {
    bool is_load(false), is_store(false), is_space(false);
    // Determine what kind of the instruction
    switch (inst.op) {
    case I_LOAD:
        is_load = true;
        break;
    case I_STORE:
        is_store = true;
        break;
    case I_NONE:
        is_space = true;
        break;
    }

    addr_t next_addr = Cvt2AddrBits(inst.addr_raw);

    this->main_cache->Ready(next_addr);

    // Handle the address
    if (is_load) {
        ++_counter.access;
        ++_counter.load;

        if (this->main_cache->IsHit()) {
            ++_counter.load_hit;
            ++_counter.hit_in_main;
        } else {
            this->main_cache->_Read(next_addr);
        }
    } else if (is_store) {
        ++_counter.access;
        ++_counter.store;

        if (this->main_cache->_IsHit(next_addr)) {
            ++_counter.store_hit;
            ++_counter.hit_in_main;

        } else {
            this->main_cache->_Read(next_addr);
        }
    } else if (is_space) {
        ++_counter.space;
    } else {
        std::cerr << "Unexpected error in _CacheHandler()" << std::endl;
        std::cerr << "ERROR line: " << inst.addr_raw << std::endl;
        return false;
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
    std::cout << "┌-------------------┐" << std::endl;
    std::cout << "│  Primary Cache    │" << std::endl;
    std::cout << "└-------------------┘" << std::endl;
    _ShowSettingInfo(this->_cache_list[0]);

    std::cout << "===================================" << std::endl;

    std::cout << "Number of cache access: " << _counter.access << std::endl;
    std::cout << "Number of cache load: " << _counter.load << std::endl;
    std::cout << "Number of cache store: " << _counter.store << std::endl;
    std::cout << "Number of total cache hit: " << _counter.hit << std::endl;

    std::cout << "Cache hit rate: " << std::setprecision(6)
              << _counter.avg_hit_rate << std::endl;
}

void Simulator::DumpCACTIConfig() {
    _DumpCACTIConfig("cacti_main.cfg", this->_cache_list[0]);
}

void Simulator::_ShowSettingInfo(const CACHE_SET &_cache_list) {
    if (_cache_list.type == VICTIM) {
        std::cout << "Cache size: " << _cache_list.cache_size << " blocks"
                  << std::endl;
    } else {
        std::cout << "Cache size: " << _cache_list.cache_size << "KB"
                  << std::endl;
    }
    std::cout << "Cache block size: " << _cache_list.block_size << "B"
              << std::endl;
    switch (_cache_list.associativity) {
    case direct_mapped:
        std::cout << "Associativity: direct_mapped" << std::endl;
        break;
    case set_associative:
        std::cout << "Associativity: " << _cache_list.cache_sets
                  << "-way set_associative" << std::endl;
        break;
    case full_associative:
        std::cout << "Associativity: fully_associative" << std::endl;
        break;
    default:
        std::cerr << "Error associtivity setting" << std::endl;
        exit(-1);
    }
    switch (_cache_list.replacement_policy) {
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

void Simulator::_DumpCACTIConfig(const std::string &filename,
                                 const CACHE_SET &_cache_list) {
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
    out_file << "-block size (bytes) " << _cache_list.block_size << std::endl;
    switch (_cache_list.associativity) {
    case direct_mapped:
        out_file << "-associativity 1\n";
        break;
    case set_associative:
        out_file << "-associativity " << _cache_list.cache_sets << std::endl;
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