#pragma once

#ifndef _CACHE_HPP_
#define _CACHE_HPP_

#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

const int MAX_LINE = 65536;

using uint = unsigned int;
using ulint = unsigned long int;

enum MAPPING_P{
    // Cache block mapping policies
    direct_mapped,
    set_associative
};

enum REPL_P{
    // Cache replacement policies
    RANDOM,
    FIFO,
    LRU
};

enum WRITE_P{
    // Cache write policies
    write_through,
    write_back
};

struct CACHE_SET{
    MAPPING_P _mapping_policy;
    REPL_P    _replacement_policy;
    WRITE_P   _write_policy;
    ulint     _cache_size;
    ulint     _line_size;
    ulint     _cache_sets;
    ulint     _num_line;
    ulint     _num_sets;
    CACHE_SET(){
        _mapping_policy     = direct_mapped;
        _replacement_policy = RANDOM;
        _write_policy       = write_back;
        _cache_size         = 256;
        _line_size          = 64;    //Bytes
        _num_line           = 16;
        _num_sets           = 0;
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
    ~Cache(){
        if(_LRU_priority){
            delete(_LRU_priority);
        }
    }
private:
    // Functions
    void _read_config();
    
    // Variables
    CACHE_SET   _cache_setting;    // Basic configurations
    COUNTER     _counter;          // Runtime statistics
    bitset<32>  _cache[MAX_LINE];  // Cache status
    // [31]: valid bit [30]: hit [29]: dirty bit [28]~[0]: data
    uint*       _LRU_priority;     // Priority table for LRU
    ulint       _current_line;     // The line being processed
    ulint       _current_set;      // The set being processed
    uint        _bit_block;        // # of bits of a block
    uint        _bit_line;         // # of bits of a line
    uint        _bit_tag;          // # of bits of a tag
    uint        _bit_set;          // # of bits of a set
};

#endif
