cmake_minimum_required(VERSION 2.8.4)

project(joedb)

if(CMAKE_COMPILER_IS_GNUCXX)
 message("== gcc")
 set(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall -pedantic -Wconversion -fno-rtti -pthread")
 set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline -fno-default-inline -fno-inline-small-functions --coverage")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
 message("== clang")
 set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fno-rtti")
endif()

if(WIN32)
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # for YouCompleteMe
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system thread)

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
 ../src/interpreter/abstract/is_identifier.cpp
 ../src/journal/File.cpp
 ../src/journal/Journal_File.cpp
)

add_library(joedb SHARED
 $<TARGET_OBJECTS:interpreter_objects>
)

add_library(io_objects OBJECT
 ../src/interpreter/io/dump.cpp
 ../src/interpreter/io/Interpreter.cpp
 ../src/interpreter/io/type_io.cpp
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
  ../test/Journal_File_Test.cpp
  ../test/StringIO_Test.cpp
  ../test/is_identifier_Test.cpp
  ../test/Test_Main.cpp
  ../test/type_io_Test.cpp
  $<TARGET_OBJECTS:interpreter_objects>
  $<TARGET_OBJECTS:io_objects>
 )
 target_link_libraries(joedb_test ${CMAKE_THREAD_LIBS_INIT})
else()
 message("== WARNING: unzip gtest-1.7.0 in the parent directory for tests")
endif()

if(Boost_SYSTEM_FOUND)
 add_executable(joedb_server ../src/server/joedb_server.cpp)
 target_link_libraries(joedb_server ${Boost_SYSTEM_LIBRARY})
 add_executable(joedb_client ../src/server/joedb_client.cpp)
 target_link_libraries(joedb_client ${Boost_SYSTEM_LIBRARY})
 target_link_libraries(joedb_client ${Boost_THREAD_LIBRARY})
else()
 message("== WARNING: boost system not found: server is not compiled")
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

#############################################################################
# Targets below require databases to be compiled
#############################################################################
add_executable(tutorial
 ../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/other_source.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)
set_target_properties(tutorial PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_executable(compiler_test
 ../test/compiler/compiler_test.cpp
 $<TARGET_OBJECTS:interpreter_objects>
)
set_target_properties(compiler_test PROPERTIES EXCLUDE_FROM_ALL TRUE)

#############################################################################
# Packaging
#############################################################################
 install(TARGETS joedbc RUNTIME DESTINATION bin)
 install(TARGETS joedbi RUNTIME DESTINATION bin)
 install(TARGETS joedb_logdump RUNTIME DESTINATION bin)
 install(TARGETS joedb ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
 # TODO: install include files = joedb/File.h ...

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
 set(CPACK_GENERATOR "DEB")
 set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Rémi Coulom")
else()
 message("== Generic package, CMAKE_SYSTEM_NAME = " ${CMAKE_SYSTEM_NAME})
endif()

include(CPack)
