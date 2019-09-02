#ifndef _CONFIG_PARSER_HPP_
#define _CONFIG_PARSER_HPP_

#include <algorithm>
#include <iostream>
#include <cstdint>
#include <list>
#include <string>
#include <fstream>
#include <cassert>
#include <iomanip>

using ulint = uint64_t;

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
    LRU
};

enum WRITE_P{
    // Cache write policies
    write_back
};

struct CACHE_SET{
    MAPPING_P associativity;
    REPL_P    replacement_policy;
    WRITE_P   write_policy;
    ulint     cache_size;          // cache size
    ulint     block_size;          // cache block size
    ulint     cache_sets;          // cache set
    ulint     num_block;           // # of lines
    ulint     num_sets;            // # of sets
    CACHE_SET(){
        associativity      = direct_mapped;
        replacement_policy = NONE;
        write_policy       = write_back;
        cache_size         = 0;
        block_size         = 0;
        cache_sets         = 0;
        num_block          = 0;
        num_sets           = 0;
    }
};
// Handle cache parameter description
CACHE_SET readConfig(char* config_filename);

// Utility functions
bool sizeCheck (ulint size);
void dumpCACTIConf(const CACHE_SET& cache_config); // Dump CACTI config file
bool readParameter(const std::string& conf, ulint& para); // Generalized read config entry
std::list<std::string> readFile(char* config_filename); // Read text file into list of string
std::list<std::string> removeComments(const std::list<std::string>& source); // Remove C/C++ style comments
#endif