#!/bin/sh
rm -rf build
mkdir build
cd build
cmake ..
make -j
./cache_sim ../TestData/gcc.trace ../TestData/cache1.cfg
./cache_sim ../TestData/gcc.trace ../TestData/cache2.cfg
./cache_sim ../TestData/gcc.trace ../TestData/cache3.cfg
