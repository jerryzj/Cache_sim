#!/bin/sh
cd build
cmake ..
make -j
./cache_sim -t ../TestData/gcc.trace -c ../TestData/cache1.cfg
./cache_sim -t ../TestData/gcc.trace -c ../TestData/cache2.cfg
./cache_sim -t ../TestData/gcc.trace -c ../TestData/cache3.cfg
