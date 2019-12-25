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
        } catch (const std::exception &ex) {
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
