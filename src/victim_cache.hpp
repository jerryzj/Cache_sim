#ifndef _VICTIM_CACHE_HPP_
#define _VICTIM_CACHE_HPP_

#include "config_parser.hpp"
#include <bitset>
#include <cassert>
#include <exception>
#include <iomanip>
#include <random>
#include <vector>

class VictimCache : public Cache {
  public:
    explicit VictimCache(const CACHE_SET &cache_setting);
    ~VictimCache();
    friend class Simulator;

  private:
    bool _IsHit(const std::bitset<32> &addr,
                const std::bitset<32> &victim_addr);
    void _Insert(const std::bitset<32> &victim_addr); // insert victim when
};

#endif