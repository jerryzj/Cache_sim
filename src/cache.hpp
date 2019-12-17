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

class Cache {
  public:
    explicit Cache(const CACHE_SET &cfg);
    explicit Cache();
    ~Cache();

    bool IsHit();

    friend class Simulator;

  protected:
    // Main handling Functions
    bool _CacheHandler(char *trace_line);       // Main Instruction processing
    bool _IsHit(const std::bitset<32> &addr);   // Data is hit/miss
    void _Read(const std::bitset<32> &addr);    // Read data from memory
    void _Drop();                               // Write data to memory
    void _Replace(const std::bitset<32> &addr); // Replace cache block
    // Predict and return potential evicted cache address in current instruction
    std::bitset<32> Ready(const std::bitset<32> &addr);

    // Utility functions
    void _Cache_Setup();
    void
    _WriteToBlock(const std::bitset<32> &addr); // Write data to cache block
    ulint _GetCacheIndex(const std::bitset<32> &addr); // Get index of block
    bool _IfBlockAvailable();

    // Check whether current address is in certain cache block
    bool _CheckIdent(const std::bitset<32> &cache, const std::bitset<32> &addr);

    // Replacement policy related
    ulint GetBlockByRandom();
    ulint GetBlockByLRU();
    // Return a memory address by given line/block
    std::bitset<32> _CvtToAddr(ulint block_set);
    void _Update();
    void _LRUHitHandle();
    // Variables
    std::bitset<32> _cache[MAX_LINE]; // Cache status
    // [31]: valid bit [30]: hit [29]: dirty bit [28]~[0]: data
    std::vector<uint> _LRU_priority; // Priority table for LRU

    // Flags
    ulint _current_block; // The block being processed
    ulint _current_set;   // The set being processed
    std::bitset<32> _current_addr;
    std::bitset<32> _poten_victim;
    bool _has_evicted;
    bool _has_space;
    bool _has_hit;
    // Cache parameters
    CACHE_SET _cache_setting; // Basic configurations
    uint _bit_block;          // # of bits of a block
    uint _bit_line;           // # of bits of a line
    uint _bit_tag;            // # of bits of a tag
    uint _bit_set;            // # of bits of a set
};

#endif
