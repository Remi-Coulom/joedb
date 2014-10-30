cmake_minimum_required(VERSION 2.8.4)

project(crazydb)

if(CMAKE_COMPILER_IS_GNUCXX)
 set(CMAKE_CXX_FLAGS "-Wall -ansi -pedantic -Wconversion")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

include_directories(
 ../src/templates
)

add_executable(
 sample
 ../src/sample/sample.cpp
)
