# Cache_sim

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/a5a7971e6230473a94f62556a8abe309)](https://www.codacy.com/manual/jerry0715no14/Cache_sim?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=jerryzj/Cache_sim&amp;utm_campaign=Badge_Grade)

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
	./cache_sim  -t ../TestData/gcc.trace -c ../TestData/cache1.json
```
