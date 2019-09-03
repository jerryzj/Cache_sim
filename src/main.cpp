#include "argparse.hpp"
#include "cache.hpp"

int main(int argc, char *argv[]) {
    ArgumentParser parser("Argument parser");
    parser.add_argument("-t", "Trace file");
    parser.add_argument("-c", "Cache config file");
    try {
        parser.parse(argc, argv);
    } catch (const ArgumentParser::ArgumentNotFound &ex) {
        std::cerr << ex.what() << std::endl;
        return -1;
    }
    if (parser.is_help())
        return 0;
    std::string config = parser.get<std::string>("c");
    std::string trace = parser.get<std::string>("t");

    Cache cache(config.c_str());
    cache.run_sim(trace.c_str());
    cache.dump_result(trace.c_str());
    cache.dump_CACTI_config();

    return 0;
}