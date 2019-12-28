#ifndef _SIMULATOR_HPP_
#define _SIMULATOR_HPP_

#include "config_parser.hpp"
#include "loader.hpp"
#include "main_cache.hpp"
#include <iomanip>
#include <memory>
#include <vector>

struct COUNTER {
    ulint access;    // # of cache access
    ulint load;      // # of load inst.
    ulint store;     // # of store inst.
    ulint space;     // # of space line
    ulint hit;       // # of hit
    ulint load_hit;  // # of load hit
    ulint store_hit; // # of store hit
    ulint hit_in_main;
    double avg_hit_rate;   // average hit rate
    double load_hit_rate;  // hit rate of loads
    double store_hit_rate; // hit rarte of stores
    double amat;           // AMAT in cycles
    explicit COUNTER()
        : access(0), load(0), store(0), space(0), hit(0), load_hit(0),
          store_hit(0), hit_in_main(0), avg_hit_rate(0.0), load_hit_rate(0.0),
          store_hit_rate(0.0) {}
};

class Simulator {
  public:
    explicit Simulator(std::vector<CacheProperty> &cache_cfg_list,
                       const std::string &program_trace,
                       const bool &multi_level_mode);
    ~Simulator();
    void RunSimulation();
    void DumpResult(const bool &oneline); // Print simulation result

  private:
    void _SetupCache(const std::vector<CacheProperty> &_cfg_list);
    bool _CacheHandler(const inst_t &inst); // Main Instruction processing
    void _Load(const addr_t &addr);
    void _Store(const addr_t &addr);
    void _CalHitRate(); // Caculate hit rate
    void _ShowSettingInfo();
    void _ShowSettingInfo(MainCache &_cache);

    std::unique_ptr<InstructionLoader> inst_loader;
    std::vector<MainCache> _cache_hierarchy_list;

    const bool _multi_level;
    const std::string &trace_file;
    COUNTER _counter;
};

#endif