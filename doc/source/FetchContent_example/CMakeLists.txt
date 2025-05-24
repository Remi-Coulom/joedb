cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)

project(FetchContent_example)

include(FetchContent)

FetchContent_Declare(joedb URL https://www.joedb.org/joedb_9.6.5.tar.gz)
FetchContent_MakeAvailable(joedb)
include(${joedb_SOURCE_DIR}/compcmake/joedbc.cmake)

joedbc_build(src example)

joedb_add_executable(example src/example.cpp)
