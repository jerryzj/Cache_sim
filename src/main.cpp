#include "argparse.hpp"
#include "simulator.hpp"

int simulator_verbose_output = 0;

int main(int argc, char **argv) {
    ArgumentParser parser("Argument parser");
    parser.add_argument("-t", "Program trace file", true);
    parser.add_argument("-c", "Cache config file", true);
    parser.add_argument("-v", "--verbose", "Verbose output", false);

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

    if (parser.exists("v")) {
        simulator_verbose_output = parser.get<int>("v");
    }

    std::string config_path = parser.get<std::string>("c");
    std::string trace_path = parser.get<std::string>("t");

    Simulator simulator(config_path, trace_path);
    simulator.RunSimulation();
    simulator.DumpResult();

    return 0;
}