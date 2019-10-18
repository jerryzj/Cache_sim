#ifndef _CONFIG_PARSER_HPP_
#define _CONFIG_PARSER_HPP_

#include "cache_setting.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

using ulint = uint64_t;

// Handle cache parameter description
CACHE_SET readConfig(const char *config_filename);

// Utility functions
bool sizeCheck(ulint size);
bool readParameter(const std::string &conf,
                   ulint &para); // Generalized read config entry
std::list<std::string>
readFile(const char *config_filename); // Read text file into list of string
std::list<std::string> removeComments(
    const std::list<std::string> &source); // Remove C/C++ style comments
#endif