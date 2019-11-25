#ifndef _CACHE_HPP_
#define _CACHE_HPP_

#include "config_parser.hpp"
#include <bitset>
#include <cassert>
#include <exception>
#include <iomanip>
#include <random>
#include <vector>

const int MAX_LINE = 65536;
using uint = unsigned int;

struct COUNTER {
    ulint access;          // # of cache access
    ulint load;            // # of load inst.
    ulint store;           // # of store inst.
    ulint space;           // # of space line
    ulint hit;             // # of hit
    ulint load_hit;        // # of load hit
    ulint store_hit;       // # of store hit
    double avg_hit_rate;   // average hit rate
    double load_hit_rate;  // hit rate of loads
    double store_hit_rate; // hit rarte of stores
    explicit COUNTER()
        : access(0), load(0), store(0), space(0), hit(0), load_hit(0),
          store_hit(0), avg_hit_rate(0.0), load_hit_rate(0.0),
          store_hit_rate(0.0) {}
};

/**
 * Convert:            Memory address                 <-   ->                               Cache 
 *                     layout(32 bits)                   |                             layout(32 bits)
 *   (MSB)                                  (LSB)        |         (MSB)                                      (LSB)
 * - Direct-Mapped:                                      |
 *   |     bit_tag      |  bit_line | bit_block |        |         | valid bit | hit | dirty bit | tag-bit [28:0] |
 * - Set-Associativity:                                  |
 *   |     bit_tag      |  bit_set  | bit_block |        |
 * - Fully-Associativity:                                |
 *   |            bit_tag           | bit_block |        |
 * 
**/
class Cache {
  public:
    explicit Cache(const char *config_filename);
    ~Cache();
    void run_sim(const char *trace_file); // Load trace file and run simulation
    void dump_result(const char *trace_file); // Print simulation result
    void dump_CACTI_config(); // Generate CACTI configuration file

  private:
    // Main handling Functions
    bool _CacheHandler(char *trace_line);       // Main Instruction processing
    bool _IsHit(const std::bitset<32> &addr);   // Data is hit/miss
    void _Read(const std::bitset<32> &addr);    // Read data from memory
    void _Drop();                               // Write data to memory
    void _Replace(const std::bitset<32> &addr); // Replace cache block

    // Utility functions
    void _Cache_Setup(); // Setup cache
    void
    _WriteToBlock(const std::bitset<32> &addr); // Write data to cache block
    ulint _GetCacheIndex(const std::bitset<32> &addr); // Get index of block
    bool _CheckIdent(const std::bitset<32> &cache, const std::bitset<32> &addr);
    // Check whether current address is in certain cache block
    void _CalHitRate(); // Caculate hit rate

    // Variables
    CACHE_SET _cache_setting;         // Basic configurations
    COUNTER _counter;                 // Runtime statistics
    std::bitset<32> _cache[MAX_LINE]; // Cache status
    // [31]: valid bit [30]: hit [29]: dirty bit [28]~[0]: data
    // std::vector<uint> _LRU_priority;    // Priority table for LRU
    ulint _current_block; // The block being processed
    ulint _current_set;   // The set being processed
    uint _bit_block;      // # of bits of a block
    uint _bit_line;       // # of bits of a line
    uint _bit_tag;        // # of bits of a tag
    uint _bit_set;        // # of bits of a set
};

#endif
