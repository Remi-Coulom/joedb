cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(minimal_example)

include(FetchContent)

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
 joedb
 GIT_REPOSITORY git@git.kayufu.com:joedb
 GIT_TAG        8674304d677b584fcfcb96a1f90f1737de0ea70e
 GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(joedb)

add_executable(minimal_example src/minimal_example.cpp)
