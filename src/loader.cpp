#include "loader.hpp"

InstructionLoader::InstructionLoader(const std::string &trace_filename) {
    LoadTraceFile(trace_filename);
}

InstructionLoader::~InstructionLoader() {
    if (in_file == nullptr) {
        in_file->close();
    }
}

void InstructionLoader::LoadTraceFile(const std::string &filename) {
    in_file = std::make_unique<std::ifstream>(filename.c_str(), std::ios::in);
    if (in_file->fail()) {
        std::cerr << "Open trace file error" << std::endl;
        exit(-1);
    }
}

inst_t InstructionLoader::GetNextInst() {
    int LENGTH_OF_INST_LINE = 13;
    char trace_line[13];
    if (IfAvailable())
        in_file->getline(trace_line, LENGTH_OF_INST_LINE);

    return _ParseLineToInst(trace_line);
}

bool InstructionLoader::IfAvailable() { return !(in_file->eof()); }

inst_t InstructionLoader::_ParseLineToInst(const char *line) {
    int INST_ADDR_BASE = 16;
    INST_OP op;
    addr_raw_t addr = (addr_raw_t)strtoul(line + 2, nullptr, INST_ADDR_BASE);

    switch (line[0]) {
    case 'l':
        // TODO: add statistics
        op = I_LOAD;
        break;
    case 's':
        // TODO: add statistics
        op = I_STORE;
        break;
    case '\0':
        // TODO: add statistics
        op = I_NONE;
        break;
    default:
        op = I_NONE;
        std::cerr << "Undefined instruction type." << std::endl;
        std::cerr << "Error line: " << line << std::endl;
    }
    return inst_t{op, addr};
}
