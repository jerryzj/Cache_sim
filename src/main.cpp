#include "argparse.hpp"
#include "simulator.hpp"

int main(int argc, char **argv) {
    ArgumentParser parser("Argument parser");
    parser.add_argument("-t", "Program trace file", true);
    parser.add_argument("-c", "Cache config file", true);
    parser.add_argument("-q", "--one-line", "Only output one-line hit rate",
                        false);

    try {
        parser.parse(argc, argv);
    } catch (const ArgumentParser::ArgumentNotFound &ex) {
        std::cerr << ex.what() << std::endl;
        parser.print_help();
        return -1;
    }

    if (parser.is_help()) {
        return 0;
    }

    std::string config_path = parser.get<std::string>("c");
    std::string trace_path = parser.get<std::string>("t");
    std::vector<CacheProperty> cache_setting_list;

    bool is_multi_level(false);
    ParseCacheConfig(config_path.c_str(), cache_setting_list, is_multi_level);

    Simulator simulator(cache_setting_list, trace_path, is_multi_level);
    simulator.RunSimulation();

    if (parser.exists("one-line")) {
        simulator.DumpResult(true);
    } else {
        simulator.DumpResult(false);
    }

    return 0;
}