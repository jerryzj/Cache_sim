rm -rf build
mkdir build
cd build
cmake ..
make -j
./cache_sim ../TestData/gcc.trace ../TestData/cache3.cfg