cmake_minimum_required(VERSION 2.8.4)

project(crazydb)

set(CMAKE_CXX_FLAGS "-O0 -std=c++11 -Wall -pedantic -Wconversion")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()

include_directories(
 ../src/interpreter/abstract
 ../src/interpreter/io
)

add_executable(
 abstract_test
 ../src/test/abstract_test.cpp
 ../src/interpreter/io/dump.cpp
)

add_executable(
 interpereter
 ../src/interpreter/io/interpreter.cpp
)
