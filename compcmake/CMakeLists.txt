cmake_minimum_required(VERSION 2.8.4)

project(joedb)

if(CMAKE_COMPILER_IS_GNUCXX)
 message("== gcc")
 set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -pedantic -Wconversion")
 set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fkeep-inline-functions -fno-inline -fno-default-inline -fno-inline-small-functions --coverage")
endif()

if(WIN32)
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # for YouCompleteMe

include_directories(
 ../src/interpreter/abstract
 ../src/interpreter/io
)

add_executable(
 interpreter
 ../src/interpreter/io/interpreter.cpp
 ../src/interpreter/io/dump.cpp
 ../src/journal/File.cpp
 ../src/journal/JournalFile.cpp
)
