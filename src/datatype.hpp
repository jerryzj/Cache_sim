#ifndef _DATATYPE_HPP_
#define _DATATYPE_HPP_
#include <bitset>
#include <fstream>
#include <iostream>

using ulint = uint64_t;
using addr_raw_t = uint64_t;
using addr_t = std::bitset<32>;

inline addr_raw_t Cvt2AddrRaw(addr_t addr) { return addr.to_ullong(); }

inline addr_t Cvt2AddrBits(addr_raw_t raw_addr) {
    return std::bitset<32>(raw_addr);
}

enum INST_OP { I_LOAD, I_STORE, I_NONE };

struct inst_t {
    INST_OP op;
    addr_raw_t addr_raw;
    inst_t() : op(I_NONE), addr_raw(0) {}
};

#endif