#ifndef _SIMULATOR_HPP_
#define _SIMULATOR_HPP_

#include "config_parser.hpp"
#include "loader.hpp"
#include "main_cache.hpp"
#include <cstdint>
#include <iomanip>
#include <memory>

struct COUNTER {
    uint64_t access;    // # of cache access
    uint64_t load;      // # of load inst.
    uint64_t store;     // # of store inst.
    uint64_t space;     // # of space line
    uint64_t hit;       // # of hit
    uint64_t load_hit;  // # of load hit
    uint64_t store_hit; // # of store hit
    uint64_t hit_in_main;
    double avg_hit_rate;   // average hit rate
    double load_hit_rate;  // hit rate of loads
    double store_hit_rate; // hit rarte of stores
    explicit COUNTER()
        : access(0), load(0), store(0), space(0), hit(0), load_hit(0),
          store_hit(0), hit_in_main(0), avg_hit_rate(0.0), load_hit_rate(0.0),
          store_hit_rate(0.0) {}
};

class Simulator {
  public:
    explicit Simulator(std::vector<CACHE_SET> &cache_cfg_list,
                       const std::string &program_trace,
                       const bool multi_level_mode);
    ~Simulator();
    void RunSimulation();
    void DumpResult(bool oneline); // Print simulation result

  private:
    std::unique_ptr<InstructionLoader> inst_loader;
    std::vector<MainCache> _cache_hierarchy_list;
    const bool _multi_level;
    const std::string &trace_file;

    void _SetupCache(std::vector<CACHE_SET> &_cfg_list);

    std::vector<CACHE_SET> _cache_setting_list;
    COUNTER _counter; // Runtime statistics

    bool _CacheHandler(inst_t inst); // Main Instruction processing
    // Check whether current address is in certain cache block
    void _Load(const addr_t &addr);
    void _Store(const addr_t &addr);

    void _CalHitRate(); // Caculate hit rate

    void _ShowSettingInfo();
    void _ShowSettingInfo(MainCache &_cache);
};

#endif