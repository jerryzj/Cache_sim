#!/bin/sh
cd build
cmake ..
make -j
./cache_sim -t ../trace/gcc.trace -c ../config/cache1.json
./cache_sim -t ../trace/gzip.trace -c ../config/cache1.json
./cache_sim -t ../trace/mcf.trace -c ../config/cache1.json
./cache_sim -t ../trace/swim.trace -c ../config/cache1.json
./cache_sim -t ../trace/twolf.trace -c ../config/cache1.json

./cache_sim -t ../trace/gcc.trace -c ../config/cache2.json
./cache_sim -t ../trace/gzip.trace -c ../config/cache2.json
./cache_sim -t ../trace/mcf.trace -c ../config/cache2.json
./cache_sim -t ../trace/swim.trace -c ../config/cache2.json
./cache_sim -t ../trace/twolf.trace -c ../config/cache2.json

./cache_sim -t ../trace/gcc.trace -c ../config/cache3.json
./cache_sim -t ../trace/gzip.trace -c ../config/cache3.json
./cache_sim -t ../trace/mcf.trace -c ../config/cache3.json
./cache_sim -t ../trace/swim.trace -c ../config/cache3.json
./cache_sim -t ../trace/twolf.trace -c ../config/cache3.json

./cache_sim -t ../trace/gcc.trace -c ../config/cache4.json
./cache_sim -t ../trace/gzip.trace -c ../config/cache4.json
./cache_sim -t ../trace/mcf.trace -c ../config/cache4.json
./cache_sim -t ../trace/swim.trace -c ../config/cache4.json
./cache_sim -t ../trace/twolf.trace -c ../config/cache4.json