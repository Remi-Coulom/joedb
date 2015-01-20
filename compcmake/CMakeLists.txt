cmake_minimum_required(VERSION 2.8.4)

project(joedb)

if(CMAKE_COMPILER_IS_GNUCXX)
 message("== gcc")
 set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pedantic -Wconversion -fno-exceptions -fno-rtti")
 set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline -fno-default-inline -fno-inline-small-functions --coverage")
endif()

if(WIN32)
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # for YouCompleteMe

enable_testing()
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

include_directories(
 ${GTEST_INCLUDE_DIRS}
 ../src/interpreter/abstract
 ../src/interpreter/io
 ../src/journal
)

add_executable(
 interpreter
 ../src/interpreter/io/interpreter.cpp
 ../src/interpreter/io/dump.cpp
 ../src/journal/File.cpp
 ../src/journal/JournalFile.cpp
)

add_executable(
 gtest
 ../test/gtest.cpp
)
target_link_libraries(gtest ${GTEST_BOTH_LIBRARIES})
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})
