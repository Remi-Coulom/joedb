cmake_minimum_required(VERSION 2.8)

project(crazydb)

if(CMAKE_COMPILER_IS_GNUCXX)
 set(CMAKE_CXX_FLAGS "-Wall -ansi -pedantic -Wconversion")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

include_directories(
 ../src/templates
)

add_executable(
 crazydbc
 ../src/compiler/crazydbc.cpp
 ../src/compiler/parse_schema.cpp
)

add_executable(
 sample
 ../src/templates/Sample.cpp
 ../src/templates/Sample.h
 ../src/sample/sample.cpp
)
