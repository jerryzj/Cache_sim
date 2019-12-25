#include "cache_setting.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void ParseCacheConfig(const char *filename, std::vector<CACHE_SET> &dest);