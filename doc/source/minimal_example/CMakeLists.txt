cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 17)

project(minimal_example)

include(FetchContent)

FetchContent_Declare(joedb URL https://www.joedb.org/joedb_9.6.4.tar.gz)
FetchContent_MakeAvailable(joedb)
include(${joedb_SOURCE_DIR}/compcmake/libjoedb.cmake)

add_executable(minimal_example src/minimal_example.cpp)
target_link_libraries(minimal_example joedb)
