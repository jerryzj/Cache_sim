#include "cache.hpp"

int main(int argc, char* argv[]){
    assert(argc >= 3);

    Cache cache(argv[2]);
    cache.run_sim(argv[1]);
    cache.dump_result(argv[1]);

    return 0;
}