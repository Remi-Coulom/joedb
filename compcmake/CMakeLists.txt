cmake_minimum_required(VERSION 2.8.4)

project(crazydb)

set(CMAKE_CXX_FLAGS "-O0 -std=c++11 -Wall -pedantic -Wconversion")

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
