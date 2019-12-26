#include "simulator.hpp"

Simulator::Simulator(std::vector<CACHE_SET> &cache_cfg_list,
                     const std::string &program_trace,
                     const bool multi_level_mode = false)
    : _multi_level(multi_level_mode), trace_file(program_trace) {

    _SetupCache(cache_cfg_list);

    inst_loader = std::make_unique<InstructionLoader>(trace_file);
}

Simulator::~Simulator() = default;

void Simulator::_SetupCache(std::vector<CACHE_SET> &_cfg_list) {
    if (_multi_level) {
        for (auto it = _cfg_list.begin(); it != _cfg_list.end(); ++it) {
            _cache_hierarchy_list.push_back(MainCache(*it));
        }
    } else {
        _cache_hierarchy_list.push_back(MainCache(_cfg_list[0]));
    }
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
    addr_t next_addr = Cvt2AddrBits(inst.addr_raw);

    // Determine what kind of the instruction
    switch (inst.op) {
    case I_LOAD:
        _Load(next_addr);
        break;
    case I_STORE:
        _Store(next_addr);
        break;
    case I_NONE:
        ++_counter.space;
        break;
    default:
        std::cerr << "Unexpected error in _CacheHandler()" << std::endl;
        std::cerr << "ERROR line: " << inst.addr_raw << std::endl;
        return false;
    }
    return true;
}

void Simulator::_Load(const addr_t &addr) {
    ++_counter.access;
    ++_counter.load;

    if (_multi_level) {
        bool _is_hit(false);
        for (auto _cache = _cache_hierarchy_list.begin();
             _cache != _cache_hierarchy_list.end() && !_is_hit; ++_cache) {
            if (_cache->Get(addr)) {
                _is_hit = true;
                ++_counter.load_hit;
                ++_counter.hit_in_main;
            } else {
                _is_hit = false;
                _cache->Set(addr);
            }
        }
    } else {
        if (_cache_hierarchy_list[0].Get(addr)) {
            ++_counter.load_hit;
            ++_counter.hit_in_main;
        } else {
            _cache_hierarchy_list[0].Set(addr);
        }
    }
}

void Simulator::_Store(const addr_t &addr) {
    ++_counter.access;
    ++_counter.store;

    if (_multi_level) {
        bool _is_hit(false);
        for (auto _cache = _cache_hierarchy_list.begin();
             _cache != _cache_hierarchy_list.end() && !_is_hit; ++_cache) {
            if (_cache->Get(addr)) {
                _is_hit = true;
                ++_counter.store_hit;
                ++_counter.hit_in_main;
            } else {
                _is_hit = false;
                _cache->Set(addr);
            }
        }
    } else {
        if (_cache_hierarchy_list[0].Get(addr)) {
            ++_counter.store_hit;
            ++_counter.hit_in_main;
        } else {
            _cache_hierarchy_list[0].Set(addr);
        }
    }
}

void Simulator::DumpResult(bool oneline) {

    // TODO: dump simulation results to yaml file,
    //       then add another yaml parser to verify correctness.
    if (oneline) {
        std::cout << std::setprecision(6) << _counter.avg_hit_rate << std::endl;
    } else {
        std::cout << "Test file: " << this->trace_file << std::endl;

        std::cout << "===================================" << std::endl;
        _ShowSettingInfo();
        std::cout << "===================================" << std::endl;

        std::cout << "Number of cache access: " << _counter.access << std::endl;
        std::cout << "Number of cache load: " << _counter.load << std::endl;
        std::cout << "Number of cache store: " << _counter.store << std::endl;
        std::cout << "Number of total cache hit: " << _counter.hit << std::endl;

        std::cout << "Cache hit rate: " << std::setprecision(6)
                  << _counter.avg_hit_rate << std::endl;
    }
}

void Simulator::_ShowSettingInfo() {
    for (std::size_t i = 0; i < _cache_hierarchy_list.size(); i++) {
        std::cout << "# L" << i + 1 << " Cache" << std::endl;
        _ShowSettingInfo(_cache_hierarchy_list[i]);
        if (i != _cache_hierarchy_list.size() - 1)
            std::cout << "-----------------------------------" << std::endl;
    }
}

void Simulator::_ShowSettingInfo(MainCache &_cache) {
    CachePropertyStruct _property = _cache.GetProperty();

    std::cout << "Cache size: " << _property._cache_size << "KB" << std::endl;

    std::cout << "Cache block size: " << _property._block_size << "B"
              << std::endl;
    switch (_property.associativity) {
    case direct_mapped:
        std::cout << "Associativity: direct-mapped" << std::endl;
        break;
    case set_associative:
        std::cout << "Associativity: " << _property._num_way
                  << "-way set_associative" << std::endl;
        break;
    case full_associative:
        std::cout << "Associativity: full-associative" << std::endl;
        break;
    default:
        std::cerr << "Error associtivity setting" << std::endl;
        exit(-1);
    }
    switch (_property.replacement_policy) {
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

void Simulator::_CalHitRate() {
    assert(_counter.access != 0);
    assert(_counter.load != 0);
    assert(_counter.store != 0);
    _counter.hit = _counter.hit_in_main;
    _counter.avg_hit_rate = static_cast<double>(_counter.hit) / _counter.access;
    _counter.load_hit_rate =
        static_cast<double>(_counter.load_hit) / _counter.load;
    _counter.store_hit_rate =
        static_cast<double>(_counter.store_hit) / _counter.store;
}
