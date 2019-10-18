#!/bin/sh
cd build
cmake ..
make -j
./cache_sim -t ../TestData/gcc.trace -c ../TestData/cache1.cfg
./cache_sim -t ../TestData/gzip.trace -c ../TestData/cache1.cfg
./cache_sim -t ../TestData/mcf.trace -c ../TestData/cache1.cfg
./cache_sim -t ../TestData/swim.trace -c ../TestData/cache1.cfg
./cache_sim -t ../TestData/twolf.trace -c ../TestData/cache1.cfg

./cache_sim -t ../TestData/gcc.trace -c ../TestData/cache2.cfg
./cache_sim -t ../TestData/gzip.trace -c ../TestData/cache2.cfg
./cache_sim -t ../TestData/mcf.trace -c ../TestData/cache2.cfg
./cache_sim -t ../TestData/swim.trace -c ../TestData/cache2.cfg
./cache_sim -t ../TestData/twolf.trace -c ../TestData/cache2.cfg

./cache_sim -t ../TestData/gcc.trace -c ../TestData/cache3.cfg
./cache_sim -t ../TestData/gzip.trace -c ../TestData/cache3.cfg
./cache_sim -t ../TestData/mcf.trace -c ../TestData/cache3.cfg
./cache_sim -t ../TestData/swim.trace -c ../TestData/cache3.cfg
./cache_sim -t ../TestData/twolf.trace -c ../TestData/cache3.cfg