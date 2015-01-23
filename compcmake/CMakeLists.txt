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

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_library(SQLITE3_LIBRARY sqlite3)

include_directories(
 ${GTEST_INCLUDE_DIRS}
 ../src/interpreter/abstract
 ../src/interpreter/io
 ../src/journal
)

add_library(interpreter_objects OBJECT
 ../src/interpreter/abstract/Database.cpp
 ../src/interpreter/abstract/Table.cpp
 ../src/interpreter/io/dump.cpp
 ../src/interpreter/io/Interpreter.cpp
 ../src/journal/File.cpp
 ../src/journal/JournalFile.cpp
)

add_executable(joedbi
 ../src/interpreter/io/interpreter_main.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)

add_executable(joedb_test
 ../test/Interpreter_Test.cpp
 ../test/File_Test.cpp
 ../test/JournalFile_Test.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)
target_link_libraries(joedb_test ${GTEST_BOTH_LIBRARIES})
target_link_libraries(joedb_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(sqlite3_insert ../benchmark/sqlite3_insert.cpp)
target_link_libraries(sqlite3_insert ${SQLITE3_LIBRARY})

add_executable(joedbi_insert
 ../benchmark/joedbi_insert.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)

add_executable(joedbc
 ../src/compiler/joedbc.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)
