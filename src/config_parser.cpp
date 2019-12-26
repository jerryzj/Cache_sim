#include "config_parser.hpp"

using json = nlohmann::json;

void ParseCacheConfig(const char *filename, std::vector<CACHE_SET> &dest,
                      bool &is_multi_level) {
    json cache_conf;
    std::ifstream file;
    try {
        file.open(filename);
        file.exceptions(std::ifstream::eofbit | std::ifstream::failbit |
                        std::ifstream::badbit);
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        exit(-1);
    }
    /*
        Parse from json file on disk.
        ref:
       https://nlohmann.github.io/json/classnlohmann_1_1basic__json_af1efc2468e6022be6e35fc2944cabe4d.html
    */
    try {
        file >> cache_conf;
        file.close();
    } catch (json::parse_error &e) {
        std::cerr << "message: " << e.what() << '\n'
                  << "exception id: " << e.id << '\n'
                  << "byte position of error: " << e.byte << std::endl;
    }

    is_multi_level = cache_conf["multi-level"];

    auto _cache_array = cache_conf["content"];

    for (auto it = _cache_array.begin(); it < _cache_array.end(); ++it) {
        CACHE_SET _c;
        // TODO: block size constraint
        try {
            _c.cache_size = (*it)["cache-size"];
            _c.block_size = (*it)["block-size"];
            std::string _str((*it)["associativity"]);
            if (_str == "direct-mapped")
                _c.associativity = direct_mapped;
            else if (_str == "full-associative")
                _c.associativity = full_associative;
            else if (_str == "set-associative") {
                _c.associativity = set_associative;
                _c.num_way = (*it)["number-of-way"];
            } else {
                std::cerr << "Unknown associativity of cache:" << '\n'
                          << _str << std::endl;
                exit(-1);
            }

            _str = (*it)["replacement-policy"];
            if (_str == "random" || _str == "RANDOM")
                _c.replacement_policy = RANDOM;
            else if (_str == "LRU" || _str == "lru")
                _c.replacement_policy = LRU;
            else {
                std::cerr << "Unknown replacement policy of cache:" << '\n'
                          << _str << std::endl;
                exit(-1);
            }
        } catch (const json::type_error &e) {
            std::cerr << "message: " << e.what() << '\n'
                      << "exception id: " << e.id << std::endl;
        }
        dest.push_back(_c);
    }
}