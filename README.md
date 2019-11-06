# Cache_sim

Cache Simulator for EE3450 course

## Package requirement

	* clang
	* cmake
	* clang-format

## Build

``` shell
cd build
cmake ..
make -j
```

Before compiling, it'll automatically format your codes using ``clang-format``.

The executable will be named as ``cache_sim``.

## Run simulation

```shell
./cache_sim  -t ../TestData/gcc.trace -c ../TestData/cache1.cfg
```
