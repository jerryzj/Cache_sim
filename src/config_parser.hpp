#ifndef _CONFIG_PARSER_HPP_
#define _CONFIG_PARSER_HPP_

#include "datatype.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void ParseCacheConfig(const char *filename, std::vector<CacheProperty> &dest,
                      bool &is_multi_level);
#endif