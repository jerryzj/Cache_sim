#ifndef _DATATYPE_HPP_
#define _DATATYPE_HPP_
#include <iostream>
using ulint = uint64_t;
using addr_raw_t = uint64_t;
using addr_t = std::bitset<32>;

inline addr_raw_t Cvt2AddrRaw(addr_t addr) { return addr.to_ullong(); }

inline addr_t Cvt2AddrBits(addr_raw_t raw_addr) {
    return std::bitset<32>(raw_addr);
}

enum INST_OP { I_LOAD, I_STORE, I_NONE };

using inst_t = struct {
    INST_OP op;
    addr_raw_t addr_raw;
};

#endif