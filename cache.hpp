#pragma once

#ifndef _CACHE_HPP_
#define _CACHE_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <cassert>

using namespace std;

const int MAX_LINE = 65536;

using uint = unsigned int;
using ulint = unsigned long int;

// This flag is used to control whether to print
// input guide when reading cache config or not
//#define PROMPT

enum MAPPING_P{
    // Cache block mapping policies
    direct_mapped,
    set_associative,
    full_associative
};

enum REPL_P{
    // Cache replacement policies
    NONE,
    RANDOM,
    FIFO,
    LRU,
    LFU
};

enum WRITE_P{
    // Cache write policies
    write_through,
    write_back
};

struct CACHE_SET{
    MAPPING_P mapping_policy;
    REPL_P    replacement_policy;
    WRITE_P   write_policy;
    ulint     cache_size;          // cache size
    ulint     line_size;           // cache line size
    ulint     cache_sets;          // cache set
    ulint     num_line;            // # of lines
    ulint     num_sets;            // # of sets
    CACHE_SET(){
        mapping_policy     = set_associative;
        replacement_policy = NONE;
        write_policy       = write_back;
        cache_size         = 64;
        line_size          = 32;    //Bytes
        num_line           = 0;
        num_sets           = 0;
    }
};

struct COUNTER{
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
    COUNTER(){
        access          = 0;
        load            = 0;
        store           = 0;
        space           = 0;
        hit             = 0;
        load_hit        = 0;
        store_hit       = 0;
        avg_hit_rate    = 0.0;
        load_hit_rate   = 0.0;
        store_hit_rate  = 0.0;
    }
};

class Cache{
public:
    Cache();
    ~Cache();
    void read_config();            // Read cache configurations
    void cache_setup();            // Setup cache
    void run_test(char* file_path);// Load trace file and run test
private:
    // Functions
    bool _cache_handler(char* address);
    void _cal_hit_rate();          // Caculate hit rate
    // Variables
    CACHE_SET   _cache_setting;    // Basic configurations
    COUNTER     _counter;          // Runtime statistics
    bitset<32>  _cache[MAX_LINE];  // Cache status
    // [31]: valid bit [30]: hit [29]: dirty bit [28]~[0]: data
    uint        _LRU_priority[MAX_LINE];// Priority table for LRU
    ulint       _current_line;     // The line being processed
    ulint       _current_set;      // The set being processed
    uint        _bit_block;        // # of bits of a block
    uint        _bit_line;         // # of bits of a line
    uint        _bit_tag;          // # of bits of a tag
    uint        _bit_set;          // # of bits of a set
};

#endif
