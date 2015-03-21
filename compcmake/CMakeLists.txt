cmake_minimum_required(VERSION 2.8.4)

project(joedb)

if(CMAKE_COMPILER_IS_GNUCXX)
 message("== gcc")
 set(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall -pedantic -Wconversion -fno-exceptions -fno-rtti")
 set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline -fno-default-inline -fno-inline-small-functions --coverage")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
 message("== clang")
 set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fno-exceptions -fno-rtti")
endif()

if(WIN32)
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # for YouCompleteMe

find_package(Threads REQUIRED)

include_directories(
 ${GTEST_INCLUDE_DIRS}
 ../src/interpreter/abstract
 ../src/interpreter/io
 ../src/journal
 ./gtest-1.7.0
 ./gtest-1.7.0/include
)

add_library(interpreter_objects OBJECT
 ../src/interpreter/abstract/Database.cpp
 ../src/interpreter/abstract/Table.cpp
 ../src/journal/File.cpp
 ../src/journal/JournalFile.cpp
)

add_library(io_objects OBJECT
 ../src/interpreter/io/dump.cpp
 ../src/interpreter/io/Interpreter.cpp
 ../src/interpreter/io/string_io.cpp
)

add_executable(joedbi
 ../src/interpreter/io/interpreter_main.cpp
 $<TARGET_OBJECTS:interpreter_objects>
 $<TARGET_OBJECTS:io_objects>
)

add_executable(joedb_logdump
 ../src/interpreter/io/joedb_logdump.cpp
 $<TARGET_OBJECTS:interpreter_objects>
 $<TARGET_OBJECTS:io_objects>
)

if(EXISTS "../gtest-1.7.0")
 add_executable(joedb_test
  ../test/Interpreter_Test.cpp
  ../test/File_Test.cpp
  ../test/JournalFile_Test.cpp
  ../test/StringIO_Test.cpp
  ../test/Test_Main.cpp
  $<TARGET_OBJECTS:interpreter_objects>
  $<TARGET_OBJECTS:io_objects>
 )
 target_link_libraries(joedb_test ${CMAKE_THREAD_LIBS_INIT})
else()
 message("== WARNING: unzip gtest-1.7.0 in the parent directory for tests")
endif()

find_library(SQLITE3_LIBRARY sqlite3)
if (SQLITE3_LIBRARY)
 add_executable(sqlite3_insert ../benchmark/sqlite3_insert.cpp)
 target_link_libraries(sqlite3_insert ${SQLITE3_LIBRARY})
else()
 message("== WARNING: install sqlite3 for sqlite3 benchmarks")
endif()

add_executable(joedbi_insert
 ../benchmark/joedbi_insert.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)

add_executable(joedbc_insert
 ../benchmark/joedbc_insert.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)
set_target_properties(joedbc_insert PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_executable(joedbc
 ../src/compiler/joedbc.cpp
 $<TARGET_OBJECTS:interpreter_objects>
 $<TARGET_OBJECTS:io_objects>
)

add_executable(tutorial
 ../doc/source/tutorial/tutorial.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)
set_target_properties(tutorial PROPERTIES EXCLUDE_FROM_ALL TRUE)
