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

if (UNIX)
 set(CMAKE_CXX_FLAGS_DEBUG
  "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address"
 )
 set(CMAKE_C_FLAGS_DEBUG
  "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address"
 )
 set(CMAKE_LINKER_FLAGS_DEBUG
  "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address"
 )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
 message("== clang")
 set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
endif()

if(WIN32)
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
 add_definitions(-DWIN32_LEAN_AND_MEAN)
 add_definitions(-DNOMINMAX)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # for YouCompleteMe
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#############################################################################
# Dependencies
#############################################################################
#add_definitions(-DJOEDB_PORTABLE)

include(CheckIncludeFileCXX)

# Threads
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(Threads REQUIRED)

# libssh
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if(TRUE)
 if(NOT ${CMAKE_SYSTEM_NAME} EQUAL CYGWIN)
  find_package(libssh QUIET)
 endif()

 if(NOT libssh_FOUND)
  unset(libssh_FOUND)
  check_include_file_cxx("libssh/libssh.h" libssh_FOUND)
 endif()

 if (libssh_FOUND)
  add_definitions(-DJOEDB_HAS_SSH)
  message("== ssh was found")
 else()
  message("== ssh not found (fix: sudo apt install libssh-dev or vcpkg install libssh:x64-windows)")
 endif()
endif()

# Google Test
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(GOOGLE_TEST ${CMAKE_CURRENT_SOURCE_DIR}/googletest-release-1.10.0/googletest)

# SQLite 3
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_library(SQLITE3_LIBRARY sqlite3)
if (SQLITE3_LIBRARY)
 add_executable(sqlite3_insert ../benchmark/sqlite3_insert.cpp)
 target_link_libraries(sqlite3_insert ${SQLITE3_LIBRARY})
else()
 message("== WARNING: install sqlite3 for sqlite3 benchmarks")
endif()

#############################################################################
# Includes
#############################################################################
include_directories(BEFORE SYSTEM
 ../src
)

include_directories(
 ../src
 ../test/compiler
 ${GOOGLE_TEST}
 ${GOOGLE_TEST}/include
)

#############################################################################
# Build Targets
#############################################################################
add_library(joedb_objects OBJECT
 ../src/joedb/Writable.cpp
 ../src/joedb/Multiplexer.cpp
 ../src/joedb/is_identifier.cpp
 ../src/joedb/external/wide_char_display_width.c
 ../src/joedb/interpreter/Database.cpp
 ../src/joedb/interpreter/Table.cpp
 ../src/joedb/io/base64.cpp
 ../src/joedb/io/Dump_Writable.cpp
 ../src/joedb/io/dump.cpp
 ../src/joedb/io/Interpreter.cpp
 ../src/joedb/io/json.cpp
 ../src/joedb/io/merge.cpp
 ../src/joedb/io/type_io.cpp
 ../src/joedb/journal/File.cpp
 ../src/joedb/journal/Generic_File.cpp
 ../src/joedb/journal/Writable_Journal.cpp
 ../src/joedb/journal/Readonly_Journal.cpp
 ../src/joedb/journal/diagnostics.cpp
 ../src/joedb/journal/Stream_File.cpp
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
 ../src/joedb/io/joedbi.cpp
)
target_link_libraries(joedbi joedb)

add_executable(joedb_ssh_connect
 ../src/joedb/io/joedb_ssh_connect.cpp
 ../src/joedb/concurrency/SSH_Connection.cpp
 ../src/joedb/concurrency/SSH_Robust_Connection.cpp
)
target_link_libraries(joedb_ssh_connect joedb)

if (libssh_FOUND)
 target_link_libraries(joedb_ssh_connect ssh)
endif()

add_executable(joedb_logdump
 ../src/joedb/io/joedb_logdump.cpp
)
target_link_libraries(joedb_logdump joedb)

add_executable(joedb_pack
 ../src/joedb/io/joedb_pack.cpp
)
target_link_libraries(joedb_pack joedb)

add_executable(joedb_merge
 ../src/joedb/io/joedb_merge.cpp
)
target_link_libraries(joedb_merge joedb)

add_executable(joedb_convert
 ../src/joedb/io/joedb_convert.cpp
 $<TARGET_OBJECTS:joedb_objects>
)

add_executable(joedb_to_json
 ../src/joedb/io/joedb_to_json.cpp
)
target_link_libraries(joedb_to_json joedb)

add_executable(file_benchmark
 ../benchmark/file_benchmark.cpp
)
target_link_libraries(file_benchmark joedb)

if(EXISTS "${GOOGLE_TEST}/include")
 add_executable(joedb_test
  ../test/endianness_Test.cpp
  ../test/diagnostics_Test.cpp
  ../test/File_Test.cpp
  ../test/Generic_File_Test.cpp
  ../test/Freedom_Keeper_Test.cpp
  ../test/Interpreter_Test.cpp
  ../test/is_identifier_Test.cpp
  ../test/Writable_Journal_Test.cpp
  ../test/Merge_Test.cpp
  ../test/Multiplexer_Test.cpp
  ../test/nested_namespace_Test.cpp
  ../test/pack_Test.cpp
  ../test/Selective_Writable_Test.cpp
  ../test/Connection_Test.cpp
  ../test/Stream_File_Test.cpp
  ../test/StringIO_Test.cpp
  ../test/Test_Main.cpp
  ../test/type_io_Test.cpp
  ../src/joedb/io/base64_decode.cpp
  ../src/joedb/compiler/nested_namespace.cpp
 )
 target_link_libraries(joedb_test ${CMAKE_THREAD_LIBS_INIT} joedb)
else()
 message("== WARNING: run ./get_gtest.sh for tests")
endif()

add_executable(joedbi_insert
 ../benchmark/joedbi_insert.cpp
)
target_link_libraries(joedbi_insert joedb)

add_executable(joedbc
 ../src/joedb/compiler/joedbc.cpp
 ../src/joedb/compiler/Compiler_Options_io.cpp
 ../src/joedb/compiler/c_wrapper.cpp
 ../src/joedb/compiler/nested_namespace.cpp
)
target_link_libraries(joedbc joedb)

add_executable(joedb_embed
 ../src/joedb/compiler/joedb_embed.cpp
 ../src/joedb/compiler/minimal_runtime_io.cpp
 ../src/joedb/io/base64.cpp
 ../src/joedb/compiler/nested_namespace.cpp
)

add_executable(joedb_file_type
 ../src/joedb/journal/Generic_File.cpp
 ../src/joedb/journal/File.cpp
 ../src/joedb/io/joedb_file_type.cpp
)

#############################################################################
# Custom commands
#############################################################################
function(joedbc_build source_dir f namespace)
 add_custom_command(
  OUTPUT
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.h
  COMMAND joedbc ${f}.joedbi ${f}.joedbc
  DEPENDS
   joedbc
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${f}.joedbi
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${f}.joedbc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}
 )
endfunction(joedbc_build)

joedbc_build(../test/compiler test testdb)
joedbc_build(../test/compiler schema_v1 schema_v1)
joedbc_build(../test/compiler schema_v2 schema_v2)
joedbc_build(../test/compiler vector_test vector_test)
joedbc_build(../test/compiler multi_index multi_index)
joedbc_build(../benchmark joedbc_insert benchmark)

function(joedbc_build_wrapper source_dir f namespace)
 add_custom_command(
  OUTPUT
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}_wrapper.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}_wrapper.h
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.h
  COMMAND joedbc ${f}.joedbi ${f}.joedbc
  DEPENDS
   joedbc
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${f}.joedbi
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${f}.joedbc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}
 )
endfunction(joedbc_build_wrapper)

joedbc_build_wrapper(../doc/source/tutorial tutorial tutorial)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/translation.h
 COMMAND generate_translation_header > translation.h
 DEPENDS
  generate_translation_header
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedb
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedb
 COMMAND ${CMAKE_COMMAND} -E remove test.joedb
 COMMAND joedbi test.joedb < test.joedbi
 DEPENDS
  joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedbi
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

#############################################################################
# joedb_embed test
#############################################################################
foreach(mode base64 escape)
 add_custom_command(
  OUTPUT
   ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb_test_${mode}.h
   ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb_test_${mode}.cpp
  COMMAND joedb_embed --${mode} test.joedb my_namespace::is_nested::testdb test_${mode}
  DEPENDS
   joedb_embed
   ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedb
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
 )
endforeach(mode)

add_executable(embedded_test
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb_test_base64.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/testdb_test_escape.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/embedded_test.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
 ../src/joedb/io/base64_decode.cpp
)

#############################################################################
# Targets below require databases to be compiled
#############################################################################
add_executable(tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/tutorial_main.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
)

add_executable(concurrency_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/concurrency_tutorial.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
)

add_executable(step_by_step_replay
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/step_by_step_replay.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
)

add_executable(file_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/file_tutorial.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
)

add_executable(stream_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/stream_tutorial.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
)

add_executable(micro_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/micro_tutorial.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
)

add_executable(index_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/index_tutorial.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
)

add_executable(wrapper_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial_wrapper.cpp
 ../doc/source/tutorial/tutorial_main.c
 ../src/joedb/compiler/minimal_runtime.cpp
)

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
 ../src/joedb/compiler/minimal_runtime.cpp
)

add_executable(joedbc_insert
 ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/benchmark.cpp
 ../benchmark/joedbc_insert.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
)

#############################################################################
# Packaging
#############################################################################
install(TARGETS joedbc RUNTIME DESTINATION bin)
install(TARGETS joedbi RUNTIME DESTINATION bin)
install(TARGETS joedb_logdump RUNTIME DESTINATION bin)
install(TARGETS joedb_pack RUNTIME DESTINATION bin)
install(TARGETS joedb_to_json RUNTIME DESTINATION bin)
install(TARGETS joedb_merge RUNTIME DESTINATION bin)
install(TARGETS joedb_embed RUNTIME DESTINATION bin)
if(libssh_FOUND)
 install(TARGETS joedb_ssh_connect RUNTIME DESTINATION bin)
endif()
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/../src/joedb/io/joedb_browser DESTINATION bin)
install(TARGETS joedb
 ARCHIVE DESTINATION lib
 LIBRARY DESTINATION lib
 RUNTIME DESTINATION bin
)
install(TARGETS joedb_static ARCHIVE DESTINATION lib)

file(GLOB_RECURSE all_includes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/../src/ ../src/joedb/*.h)
foreach(file ${all_includes})
 get_filename_component(dir ${file} PATH)
 install(FILES ../src/${file} DESTINATION include/${dir})
endforeach()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
 set(CPACK_GENERATOR "DEB")
 set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Rémi Coulom")
else()
 message("== Generic package, CMAKE_SYSTEM_NAME = " ${CMAKE_SYSTEM_NAME})
endif()

include(CPack)
