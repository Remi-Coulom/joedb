cmake_minimum_required(VERSION 2.8.4)

file(STRINGS "../VERSION" QUOTED_JOEDB_VERSION)
string(REPLACE "\"" "" JOEDB_VERSION ${QUOTED_JOEDB_VERSION})
message("== JOEDB_VERSION = ${JOEDB_VERSION}")

project(joedb)

if(CMAKE_COMPILER_IS_GNUCXX)
 message("== gcc")
 set(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall -pedantic -Wconversion -pthread")
 set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 -fno-inline -fno-default-inline -fno-inline-small-functions --coverage")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
 message("== clang")
 set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
endif()

if(WIN32)
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # for YouCompleteMe
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Threads REQUIRED)

set(GOOGLE_TEST ${CMAKE_CURRENT_SOURCE_DIR}/googletest-release-1.8.0/googletest)

include_directories(
 ${GTEST_INCLUDE_DIRS}
 ../src/external
 ../src/includes
 ../src/includes/joedb
 ../src/interpreter/abstract
 ../src/interpreter/io
 ../src/journal
 ../../../emsdk_portable/emscripten/master/system/include
 ${GOOGLE_TEST}
 ${GOOGLE_TEST}/include
)

add_library(joedb_objects OBJECT
 ../src/external/wide_char_display_width.c
 ../src/interpreter/abstract/Database.cpp
 ../src/interpreter/abstract/Table.cpp
 ../src/interpreter/abstract/is_identifier.cpp
 ../src/interpreter/abstract/Writeable.cpp
 ../src/interpreter/abstract/Multiplexer.cpp
 ../src/interpreter/io/base64.cpp
 ../src/interpreter/io/Dump_Writeable.cpp
 ../src/interpreter/io/dump.cpp
 ../src/interpreter/io/Interpreter.cpp
 ../src/interpreter/io/json.cpp
 ../src/interpreter/io/type_io.cpp
 ../src/journal/File.cpp
 ../src/journal/Generic_File.cpp
 ../src/journal/Journal_File.cpp
 ../src/journal/Readonly_Journal.cpp
 ../src/journal/diagnostics.cpp
 ../src/journal/Stream_File.cpp
)

# The shared library does not work with Visual Studio: link statically
if (UNIX)
 add_library(joedb SHARED
  $<TARGET_OBJECTS:joedb_objects>
 )
 set_target_properties(joedb PROPERTIES SOVERSION ${JOEDB_VERSION})
else()
 add_library(joedb STATIC
  $<TARGET_OBJECTS:joedb_objects>
 )
endif()

add_library(joedb_static STATIC
 $<TARGET_OBJECTS:joedb_objects>
)

add_executable(joedbi
 ../src/interpreter/io/joedbi.cpp
)
target_link_libraries(joedbi joedb)

add_executable(joedb_logdump
 ../src/interpreter/io/joedb_logdump.cpp
)
target_link_libraries(joedb_logdump joedb)

add_executable(joedb_pack
 ../src/interpreter/io/joedb_pack.cpp
)
target_link_libraries(joedb_pack joedb)

add_executable(joedb_convert
 ../src/interpreter/io/joedb_convert.cpp
 $<TARGET_OBJECTS:joedb_objects>
)

add_executable(joedb_to_json
 ../src/interpreter/io/joedb_to_json.cpp
)
target_link_libraries(joedb_to_json joedb)

add_executable(file_benchmark
 ../benchmark/file_benchmark.cpp
)
target_link_libraries(file_benchmark joedb)

if(EXISTS "${GOOGLE_TEST}/include")
 add_executable(joedb_test
  ../test/endianness_Test.cpp
  ../test/File_Test.cpp
  ../test/Freedom_Keeper_Test.cpp
  ../test/Interpreter_Test.cpp
  ../test/is_identifier_Test.cpp
  ../test/Journal_File_Test.cpp
  ../test/Multiplexer_Test.cpp
  ../test/pack_Test.cpp
  ../test/Selective_Writeable_Test.cpp
  ../test/Stream_File_Test.cpp
  ../test/StringIO_Test.cpp
  ../test/Test_Main.cpp
  ../test/type_io_Test.cpp
 )
 target_link_libraries(joedb_test ${CMAKE_THREAD_LIBS_INIT} joedb)
else()
 message("== WARNING: run ./get_gtest.sh for tests")
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
)
target_link_libraries(joedbi_insert joedb)

add_executable(joedbc
 ../src/compiler/joedbc.cpp
 ../src/compiler/Compiler_Options_io.cpp
 ../src/compiler/c_wrapper.cpp
)
target_link_libraries(joedbc joedb)

#############################################################################
# Databases that require compilation with joedbc
#############################################################################
add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial_wrapper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial_wrapper.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.h
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/joedbc tutorial.joedbi tutorial.joedbc
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/joedbc
  ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.joedbc
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb.h
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/joedbc test.joedbi test.joedbc
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/joedbc
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedbc
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v1.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v1.h
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/joedbc schema_v1.joedbi schema_v1.joedbc
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/joedbc
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v1.joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v1.joedbc
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v2.h
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/joedbc schema_v2.joedbi schema_v2.joedbc
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/joedbc
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v2.joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v2.joedbc
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/vector_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/vector_test.h
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/joedbc vector_test.joedbi vector_test.joedbc
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/joedbc
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/vector_test.joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/vector_test.joedbc
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/multi_index.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/multi_index.h
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/joedbc multi_index.joedbi multi_index.joedbc
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/joedbc
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/multi_index.joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/multi_index.joedbc
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/benchmark.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/benchmark.h
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/joedbc joedbc_insert.joedbi joedbc_insert.joedbc
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/joedbc
  ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/joedbc_insert.joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/joedbc_insert.joedbc
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/translation.h
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/generate_translation_header >translation.h
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/generate_translation_header
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedb
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedb
 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/joedbi test.joedb <test.joedbi
 DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedbi
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

#############################################################################
# Targets below require databases to be compiled
#############################################################################
add_executable(tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/tutorial_main.cpp
)
target_link_libraries(tutorial joedb)

add_executable(index_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/index_tutorial.cpp
)
target_link_libraries(index_tutorial joedb)

add_executable(wrapper_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial_wrapper.cpp
 ../doc/source/tutorial/tutorial_main.c
)
target_link_libraries(wrapper_tutorial joedb)

add_executable(compiler_test
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v1.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/schema_v2.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/vector_test.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/multi_index.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/translation.h
 ../test/compiler/compiler_test.cpp
)
target_link_libraries(compiler_test joedb)

add_executable(generate_translation_header
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb.cpp
 ../test/compiler/generate_translation_header.cpp
)
target_link_libraries(generate_translation_header joedb)

add_executable(joedbc_insert
 ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/benchmark.cpp
 ../benchmark/joedbc_insert.cpp
)
target_link_libraries(joedbc_insert joedb)

#############################################################################
# Packaging
#############################################################################
install(TARGETS joedbc RUNTIME DESTINATION bin)
install(TARGETS joedbi RUNTIME DESTINATION bin)
install(TARGETS joedb_logdump RUNTIME DESTINATION bin)
install(TARGETS joedb_pack RUNTIME DESTINATION bin)
install(TARGETS joedb_to_json RUNTIME DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/../src/interpreter/io/joedb_browser DESTINATION bin)
install(TARGETS joedb
 ARCHIVE DESTINATION lib
 LIBRARY DESTINATION lib
 RUNTIME DESTINATION bin
)
install(TARGETS joedb_static ARCHIVE DESTINATION lib)

file(GLOB all_includes ../src/includes/joedb/*.h)
install(FILES ${all_includes} DESTINATION include/joedb)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
 set(CPACK_GENERATOR "DEB")
 set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Rémi Coulom")
else()
 message("== Generic package, CMAKE_SYSTEM_NAME = " ${CMAKE_SYSTEM_NAME})
endif()

include(CPack)
