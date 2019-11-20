#include "argparse.hpp"
//#include "cache.hpp"
#include "simulator.hpp"

int main(int argc, char *argv[]) {
    ArgumentParser parser("Argument parser");
    parser.add_argument("-t", "Program trace file");
    parser.add_argument("-c", "Cache config file");

    try {
        parser.parse(argc, argv);
    } catch (const ArgumentParser::ArgumentNotFound &ex) {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    if (parser.is_help()) {
        return 0;
    }

    std::string config_path = parser.get<std::string>("c");
    std::string trace_path = parser.get<std::string>("t");

    Simulator simulator(config_path, trace_path);

    simulator.RunSimulation();
    simulator.DumpResult();
    // Cache cache(config_path.c_str());
    // cache.run_sim(trace_path.c_str());
    // cache.dump_result(trace_path.c_str());
    // cache.dump_CACTI_config();

    return 0;
}