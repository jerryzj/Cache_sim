cmake_minimum_required(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ClangFormat)

set(PROJECT_NAME cache_sim)
project(${PROJECT_NAME} CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

## compiler option setting
set(CMAKE_BUILD_TYPE "DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pedantic -Wall -fsanitize=undefined,address -Werror -Wextra -Wno-unused-variable -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_RELEASE "-Oz")
## Use C++17 std
set(CMAKE_CXX_STANDARD 17)
## Use clang instead of gcc
set(CMAKE_CXX_COMPILER "clang++")

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.hpp")
clangformat_setup(${SOURCES} ${HEADERS})

add_executable(cache_sim ${SOURCES})
add_dependencies(cache_sim clangformat)
