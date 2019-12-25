#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "cache.hpp"
#include "cache_parser.hpp"
#include "loader.hpp"
#include <cstdint>
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
    uint64_t hit_in_victim;
    double avg_hit_rate;   // average hit rate
    double load_hit_rate;  // hit rate of loads
    double store_hit_rate; // hit rarte of stores
    explicit COUNTER()
        : access(0), load(0), store(0), space(0), hit(0), load_hit(0),
          store_hit(0), hit_in_main(0), hit_in_victim(0), avg_hit_rate(0.0),
          load_hit_rate(0.0), store_hit_rate(0.0) {}
};

class Simulator {
  public:
    explicit Simulator(const std::string &cache_cfg,
                       const std::string &program_trace);
    ~Simulator();
    void RunSimulation();
    void DumpResult();      // Print simulation result
    void DumpCACTIConfig(); // Generate CACTI configuration file
    void ReadConfig();

  private:
    std::unique_ptr<InstructionLoader> inst_loader;
    std::unique_ptr<Cache> main_cache;
    const std::string &cache_cfg_file;
    const std::string &trace_file;
    bool _has_victim;
    std::vector<CACHE_SET> _cache_list;
    COUNTER _counter; // Runtime statistics

    bool _CacheHandler(inst_t inst); // Main Instruction processing
    uint64_t _GetCacheIndex(const std::bitset<32> &addr); // Get index of block
    bool _CheckIdent(const std::bitset<32> &cache, const std::bitset<32> &addr);
    // Check whether current address is in certain cache block
    void _CalHitRate();                         // Caculate hit rate
    bool _IsHit(const std::bitset<32> &addr);   // Data is hit/miss
    void _Read(const std::bitset<32> &addr);    // Read data from memory
    void _Drop();                               // Write data to memory
    void _Replace(const std::bitset<32> &addr); // Replace cache block
    void _DumpCACTIConfig(const std::string &, const CACHE_SET &);
    void _ShowSettingInfo(const CACHE_SET &setting);
};

#endif