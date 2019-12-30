#ifndef _DATATYPE_HPP_
#define _DATATYPE_HPP_

#include <bitset>
#include <fstream>
#include <iostream>
#define ADDR_WIDTH 48
using ulint = uint64_t;
using addr_raw_t = uint64_t;
using addr_t = std::bitset<ADDR_WIDTH>;

inline addr_raw_t Cvt2AddrRaw(addr_t addr) { return addr.to_ullong(); }

inline addr_t Cvt2AddrBits(addr_raw_t raw_addr) { return addr_t(raw_addr); }

enum INST_OP { I_LOAD, I_STORE, I_NONE };

struct inst_t {
    INST_OP op;
    addr_raw_t addr_raw;
    explicit inst_t() : op(I_NONE), addr_raw(0) {}
};

enum MappingPolicies {
    // Cache block mapping policies
    direct_mapped,
    set_associative,
    full_associative
};

enum ReplacePolicies {
    // Cache replacement policies
    NONE,
    RANDOM,
    LRU
};

enum WritePolicies {
    // Cache write policies
    write_back
};

struct CacheProperty {
    MappingPolicies associativity;
    ReplacePolicies replacement_policy;
    WritePolicies write_policy;

    ulint _cache_size;
    ulint _block_size;

    ulint _bit_offset; // # of bits of offset
    ulint _bit_index;  // # of bits of index
    ulint _bit_set;    // # of bits of set
    ulint _bit_tag;    // # of bits of tag

    ulint _num_block; // # of blocks
    ulint _num_way;   // N-way
    ulint _num_set;   // # of sets

    explicit CacheProperty()
        : associativity(direct_mapped), replacement_policy(NONE),
          write_policy(write_back), _cache_size(0), _block_size(0),
          _bit_offset(0), _bit_index(0), _bit_set(0), _bit_tag(0),
          _num_block(0), _num_way(0), _num_set(0) {}
};

#endif