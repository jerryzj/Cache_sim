#ifndef _LOADER_HPP_
#define _LOADER_HPP_

#include "datatype.hpp"
#include <fstream>
#include <iostream>

class InstructionLoader {
  public:
    InstructionLoader(const std::string &filename);
    InstructionLoader() {}
    ~InstructionLoader();
    void LoadTraceFile(const std::string &filename);
    inst_t GetNextInst();
    bool IfAvailable();

  private:
    inst_t _ParseLineToInst(const char *line);
    std::unique_ptr<std::ifstream> in_file;
};

#endif