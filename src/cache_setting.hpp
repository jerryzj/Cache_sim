#ifndef CACHE_SETTING_HPP
#define CACHE_SETTING_HPP

#include <cstdint>

using ulint = uint64_t;

enum MAPPING_P {
    // Cache block mapping policies
    direct_mapped,
    set_associative,
    full_associative
};

enum REPL_P {
    // Cache replacement policies
    NONE,
    RANDOM,
    LRU
};

enum WRITE_P {
    // Cache write policies
    write_back
};

struct CACHE_SET {
    MAPPING_P associativity;
    REPL_P replacement_policy;
    WRITE_P write_policy;
    ulint type;       // type of cache: (0) main cache (1) victim cache
    ulint cache_size; // cache size
    ulint block_size; // cache block size
    ulint cache_sets; // cache set
    ulint num_block;  // # of lines
    ulint num_sets;   // # of sets
    CACHE_SET()
        : associativity(direct_mapped), replacement_policy(NONE),
          write_policy(write_back), cache_size(0), block_size(0), cache_sets(0),
          num_block(0), num_sets(0) {}
};

#endif