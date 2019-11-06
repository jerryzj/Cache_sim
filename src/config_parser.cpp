#include "config_parser.hpp"

CACHE_SET readConfig(const char *config_filename) {
    CACHE_SET _cache_conf;
    std::list<std::string> config_raw, config_ready;

    // Preprocessing config file
    config_raw = readFile(config_filename);
    config_ready = removeComments(config_raw);

    // Assert config lines is in the right range
    assert(config_ready.size() >= 3 && config_ready.size() <= 5);

    // Read cache size
    assert(readParameter(config_ready.front(), _cache_conf.cache_size));
    config_ready.pop_front();

    // Read cache line size
    assert(readParameter(config_ready.front(), _cache_conf.block_size));
    config_ready.pop_front();

    // Read cache associativity
    switch (stoi(config_ready.front())) {
    case 1:
        _cache_conf.associativity = direct_mapped;
        break;
    case 2:
        _cache_conf.associativity = set_associative;
        break;
    case 3:
        _cache_conf.associativity = full_associative;
        break;
    default:
        std::cerr << "Invalid Associativity" << std::endl;
        std::exit(-1);
    }
    config_ready.pop_front();

    if (_cache_conf.associativity == direct_mapped) {
        // Note that replacement policy is invalid for direct-mapped cache
        _cache_conf.replacement_policy = NONE;
        assert(config_ready.empty());
        return _cache_conf;
    } else if (_cache_conf.associativity == set_associative) {
        // Read set size
        assert(readParameter(config_ready.front(), _cache_conf.cache_sets));
        config_ready.pop_front();
    }
    // Read replacement policy
    switch (stoi(config_ready.front())) {
    case 1:
        _cache_conf.replacement_policy = RANDOM;
        break;
    case 2:
        _cache_conf.replacement_policy = LRU;
        break;
    // Add your own policy here, ex: RRIP
    default:
        std::cerr << "Invalid replacement policy" << std::endl;
        std::exit(-1);
    }
    config_ready.pop_front();

    assert(config_ready.empty());
    return _cache_conf;
}

bool sizeCheck(ulint size) {
    return ~((size < 1) || (size >= 262144) || (size & (~size + 1)) != size);
}

bool readParameter(const std::string &conf, ulint &para) {
    para = std::stoull(conf.c_str());
    if (sizeCheck(para)) {
        return true;
    }
    return false;
}

std::list<std::string> readFile(const char *config_filename) {
    std::ifstream file(config_filename, std::ios::in);
    std::string temp;
    std::list<std::string> ans;

    if (file.fail()) {
        std::cerr << "Open config file error" << std::endl;
        exit(-1);
    }
    while (getline(file, temp)) {
        temp.erase(remove(temp.begin(), temp.end(), '\r'), temp.end());
        ans.push_back(temp);
    }
    file.close();
    return ans;
}

std::list<std::string> removeComments(const std::list<std::string> &source) {
    std::list<std::string> ans;
    bool status(false); // false:nothing, true:under block comment

    for (auto line : source) {
        if (status == false && (ans.empty() || ans.back() != "")) {
            ans.push_back("");
        }
        int i(0);
        while (i < static_cast<int>(line.size())) {
            if (line.substr(i, 2) == "//") {
                if (status == false)
                    break; // break  while(i<line.size()){...}   loop
            } else if (line.substr(i, 1) == "#") {
                if (status == false)
                    break; // break  while(i<line.size()){...}   loop
            } else if (line.substr(i, 2) == "/*") {
                if (status == false) {
                    status = true;
                    i += 2;
                    continue;
                }
            } else if (line.substr(i, 2) == "*/") {
                if (status == true) {
                    status = false;
                    i += 2;
                    continue;
                }
            }
            if (status == false) {
                ans.back().push_back(line[i]);
            }
            i++;
        }
    }
    return ans;
}