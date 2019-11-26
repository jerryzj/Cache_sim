#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "cache.hpp"
#include "victim_cache.hpp"
#include "config_parser.hpp"
#include <cstdint>
#include <vector>

struct COUNTER {
    uint64_t access;       // # of cache access
    uint64_t load;         // # of load inst.
    uint64_t store;        // # of store inst.
    uint64_t space;        // # of space line
    uint64_t hit;          // # of hit
    uint64_t load_hit;     // # of load hit
    uint64_t store_hit;    // # of store hit
    double avg_hit_rate;   // average hit rate
    double load_hit_rate;  // hit rate of loads
    double store_hit_rate; // hit rarte of stores
    explicit COUNTER()
        : access(0), load(0), store(0), space(0), hit(0), load_hit(0),
          store_hit(0), avg_hit_rate(0.0), load_hit_rate(0.0),
          store_hit_rate(0.0) {}
};

class Simulator {
  public:
    Simulator::Simulator(std::string &cache_cfg, std::string &program_trace);
    Simulator::~Simulator();
    // void run_sim(const char *trace_file);       // Load trace file and run
    // simulation
    void RunSimulation();
    void DumpResult();        // Print simulation result
    void dump_CACTI_config(); // Generate CACTI configuration file
    void LoadProgram(std::string &trace_file) { this->trace_file = trace_file; }

    void ParseCacheConfig();
    void ReadConfig();
    void CacheSetup();

  private:
    Cache *main_cache;
    VictimCache *victim_cache;
    std::string trace_file;
    std::string cache_cfg_file;

    bool _has_victim;
    CACHE_SET _cache_setting;
    CACHE_SET _victim_setting;
    COUNTER _counter; // Runtime statistics

    bool _CacheHandler(char *trace_line); // Main Instruction processing
    uint64_t _GetCacheIndex(const std::bitset<32> &addr); // Get index of block
    bool _CheckIdent(const std::bitset<32> &cache, const std::bitset<32> &addr);
    // Check whether current address is in certain cache block
    void _CalHitRate();                         // Caculate hit rate
    bool _IsHit(const std::bitset<32> &addr);   // Data is hit/miss
    void _Read(const std::bitset<32> &addr);    // Read data from memory
    void _Drop();                               // Write data to memory
    void _Replace(const std::bitset<32> &addr); // Replace cache block
};

#endif