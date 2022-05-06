cmake_minimum_required(VERSION 3.9)
#cmake_minimum_required(VERSION 3.3)
#For some reasons requiring 3.9 is necessary in MSVC

file(STRINGS "../VERSION" QUOTED_JOEDB_VERSION)
string(REPLACE "\"" "" JOEDB_VERSION ${QUOTED_JOEDB_VERSION})

project(joedb VERSION ${JOEDB_VERSION})

message("== JOEDB_VERSION = ${JOEDB_VERSION}; CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # for YouCompleteMe
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

enable_testing()
add_test(NAME build_all
 COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --config "$<CONFIG>"
)

#############################################################################
if(CMAKE_COMPILER_IS_GNUCXX)
#############################################################################
 message("== gcc")
 set(CMAKE_CXX_FLAGS
  "-Wall -Wextra -Wno-unused-parameter -Wduplicated-cond -Wshadow=local -pedantic -Wconversion -Wstack-usage=131072 -Wunused-macros -Wc++14-compat -Wc++17-compat -Wcast-qual -Wcast-align -Wparentheses -Wlogical-op -Wmissing-declarations -Wredundant-decls"
 )
 set(CMAKE_CXX_FLAGS_COVERAGE
  "-g -O0 -fno-inline -fno-default-inline -fno-inline-small-functions --coverage"
 )
endif()

#############################################################################
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#############################################################################
 message("== clang")

 find_program(HAS_CLANG_TIDY clang-tidy)
 if (HAS_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy --header-filter=* -checks=-*,readability-*,-readability-braces-around-statements,-readability-magic-numbers,-readability-implicit-bool-conversion,-readability-else-after-return,-readability-uppercase-literal-suffix,-readability-static-accessed-through-instance,bugprone-*,-bugprone-macro-parentheses,-bugprone-exception-escape,-bugprone-branch-clone,concurrency-*,modernize-*,-modernize-use-trailing-return-type,-modernize-use-auto,-modernize-raw-string-literal,-modernize-avoid-c-arrays,-modernize-deprecated-headers,-modernize-loop-convert,-modernize-return-braced-init-list,-modernize-use-default-member-init,-modernize-use-using)
 else()
  message("-- no clang-tidy")
 endif()

 set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wunused-macros -Wcast-qual -Wcast-align -Wparentheses -Wmissing-declarations")

 if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow-field-in-constructor-modified -Wshadow-uncaptured-local -Wshadow -Wshadow-ivar")
 endif()

 set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

#############################################################################
if (UNIX AND NOT ${CMAKE_SYSTEM_NAME} EQUAL CYGWIN)
#############################################################################
#  "-fsanitize=memory" unfortunately triggers many errors in the standard lib
#  set(JOEDB_ASAN_FLAGS "-fno-omit-frame-pointer -fsanitize=memory")
 set(JOEDB_ASAN_FLAGS "-O3 --param=max-vartrack-size=1000000000 -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=address,undefined")
endif()

set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} ${JOEDB_ASAN_FLAGS}")
set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_DEBUG} ${JOEDB_ASAN_FLAGS}")
set(CMAKE_LINKER_FLAGS_ASAN "${CMAKE_LINKER_FLAGS_DEBUG} ${JOEDB_ASAN_FLAGS}")

#############################################################################
if(WIN32)
#############################################################################
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
 add_definitions(-DWIN32_LEAN_AND_MEAN)
 add_definitions(-DNOMINMAX)
endif()

#############################################################################
# Custom add_executable
#############################################################################
if (CMAKE_VERSION VERSION_GREATER 3.9)
 cmake_policy(SET CMP0069 NEW)
 include(CheckIPOSupported)
 check_ipo_supported(RESULT ipo_supported)
endif()

if (ipo_supported)
 message("-- IPO supported")
else()
 message("-- IPO not supported")
endif()

function(ipo_add_executable)
 add_executable(${ARGV})
 if (ipo_supported)
  set_property(TARGET ${ARGV0} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
 endif()
endfunction(ipo_add_executable)

function(joedb_add_executable)
 ipo_add_executable(${ARGV})
 target_link_libraries(${ARGV0} joedb)
endfunction(joedb_add_executable)

#############################################################################
# Dependencies
#############################################################################
if (DEFINED JOEDB_PORTABLE)
 message("-- JOEDB_PORTABLE")
 add_definitions(-DJOEDB_PORTABLE)
endif()

include(CheckIncludeFileCXX)

# Threads
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# libssh
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
include("libssh.cmake")

# Google Test
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(GOOGLE_TEST
 ${CMAKE_CURRENT_SOURCE_DIR}/../submodules/googletest/googletest
)

# SQLite 3
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_library(SQLITE3_LIBRARY sqlite3)
if (SQLITE3_LIBRARY)
 ipo_add_executable(sqlite3_insert ../benchmark/sqlite3_insert.cpp)
 target_link_libraries(sqlite3_insert ${SQLITE3_LIBRARY})
else()
 message("== WARNING: install sqlite3 for sqlite3 benchmarks")
endif()

# Networking
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(ASIO_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../submodules/asio/asio/include)
if (TRUE)
 if (EXISTS "${ASIO_DIRECTORY}/asio/ts/net.hpp")
  message("-- Found asio in submodules")
  include_directories(${ASIO_DIRECTORY})
  add_definitions(-DJOEDB_HAS_ASIO_NET)
  set(HAS_NETWORKING TRUE)
  message("== networking OK")
 else()
  message("== no networking. Try git submodule update --init --recursive")
 endif()
endif()

#############################################################################
# Includes
#############################################################################
include_directories(BEFORE SYSTEM
 ../src
)

include_directories(
 ../test/compiler
 ${GOOGLE_TEST}
 ${GOOGLE_TEST}/include
)

#############################################################################
# Build Targets
#############################################################################
set(JOEDB_SOURCES
 ../src/external/wide_char_display_width.cpp
 ../src/joedb/Readable.cpp
 ../src/joedb/Writable.cpp
 ../src/joedb/Multiplexer.cpp
 ../src/joedb/is_identifier.cpp
 ../src/joedb/Destructor_Logger.cpp
 ../src/joedb/get_version.cpp
 ../src/joedb/interpreter/Database.cpp
 ../src/joedb/interpreter/Database_Schema.cpp
 ../src/joedb/interpreter/Table.cpp
 ../src/joedb/io/base64.cpp
 ../src/joedb/io/get_time_string.cpp
 ../src/joedb/io/dump.cpp
 ../src/joedb/io/Interpreter.cpp
 ../src/joedb/io/Interpreter_Dump_Writable.cpp
 ../src/joedb/io/json.cpp
 ../src/joedb/io/merge.cpp
 ../src/joedb/io/Raw_Dump_Writable.cpp
 ../src/joedb/io/SQL_Dump_Writable.cpp
 ../src/joedb/io/type_io.cpp
 ../src/joedb/journal/File.cpp
 ../src/joedb/journal/Generic_File.cpp
 ../src/joedb/journal/Memory_File.cpp
 ../src/joedb/journal/Writable_Journal.cpp
 ../src/joedb/journal/Readonly_Journal.cpp
 ../src/joedb/journal/diagnostics.cpp
 ../src/joedb/journal/Stream_File.cpp
 ../src/joedb/journal/SHA_256.cpp
 ../src/joedb/journal/Interpreted_File.cpp
 ../src/joedb/concurrency/Server_Connection.cpp
)

if (HAS_NETWORKING)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ../src/joedb/concurrency/Server.cpp
  ../src/joedb/concurrency/Network_Channel.cpp
 )
endif()

if(libssh_FOUND)
 set(JOEDB_SOURCES ${JOEDB_SOURCES}
  ../src/joedb/ssh/Forward_Channel.cpp
 )
endif()

# The shared library does not work with Visual Studio: link statically
if (UNIX)
 add_library(joedb SHARED ${JOEDB_SOURCES})
 set_target_properties(joedb PROPERTIES SOVERSION ${JOEDB_VERSION})
 if(HAS_NETWORKING AND ${CMAKE_SYSTEM_NAME} EQUAL CYGWIN)
  target_link_libraries(joedb wsock32 ws2_32)
 endif()
else()
 add_library(joedb STATIC ${JOEDB_SOURCES})
endif()
target_link_libraries(joedb Threads::Threads)
if(libssh_FOUND)
 target_link_libraries(joedb ${LIBSSH_LIBRARIES})
endif()

joedb_add_executable(joedbi ../src/joedb/io/joedbi.cpp)
joedb_add_executable(joedb_logdump ../src/joedb/io/joedb_logdump.cpp)
joedb_add_executable(joedb_pack ../src/joedb/io/joedb_pack.cpp)
joedb_add_executable(joedb_merge ../src/joedb/io/joedb_merge.cpp)
joedb_add_executable(joedb_to_json ../src/joedb/io/joedb_to_json.cpp)
joedb_add_executable(file_benchmark ../benchmark/file_benchmark.cpp)

ipo_add_executable(joedb_convert
 ../src/joedb/io/joedb_convert.cpp
 ../src/joedb/Destructor_Logger.cpp
 ../src/joedb/Writable.cpp
 ../src/joedb/journal/File.cpp
 ../src/joedb/journal/Stream_File.cpp
 ../src/joedb/journal/Generic_File.cpp
 ../src/joedb/journal/Readonly_Journal.cpp
 ../src/joedb/journal/SHA_256.cpp
 ../src/joedb/journal/Writable_Journal.cpp
)

ipo_add_executable(joedb_sha256sum
 ../src/joedb/io/joedb_sha256sum.cpp
 ../src/joedb/journal/SHA_256.cpp
)

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
  ../test/Polymorphic_File_Test.cpp
  ../test/Selective_Writable_Test.cpp
  ../test/Client_Test.cpp
  ../test/Stream_File_Test.cpp
  ../test/StringIO_Test.cpp
  ../test/Test_Main.cpp
  ../test/type_io_Test.cpp
  ../test/Journal_Test.cpp
  ../test/Server_Connection_Test.cpp
  ../test/SHA_256_Test.cpp
  ../test/Mutex_Test.cpp
  ../test/Span_Test.cpp
  ../test/Local_Connection_Test.cpp
  ../test/compiler/Compiler_GTest.cpp
  ../test/compiler/embedded_GTest.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/multi_index.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/schema_v1.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/schema_v2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/vector_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/empty.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/translation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/test_dbname_base64.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/test_dbname_escape.cpp
  ../src/joedb/io/base64_decode.cpp
  ../src/joedb/compiler/nested_namespace.cpp
  ../src/joedb/journal/Portable_File.cpp
  ../src/joedb/String_Logger.cpp
 )
 target_link_libraries(joedb_test joedb)

 if (HAS_NETWORKING)
  target_sources(joedb_test PRIVATE ../test/Server_Test.cpp)
 endif()

 # This definition is necessary for cygwin
 # https://github.com/google/googletest/pull/2839
 target_compile_definitions(joedb_test PUBLIC _GNU_SOURCE)

 add_test(NAME joedb_test
  COMMAND joedb_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test
 )
 SET_TESTS_PROPERTIES(joedb_test PROPERTIES DEPENDS build_all)
else()
 message("== WARNING: googletest is missing")
endif()

joedb_add_executable(joedbi_insert ../benchmark/joedbi_insert.cpp)
joedb_add_executable(joedbc
 ../src/joedb/compiler/joedbc.cpp
 ../src/joedb/compiler/Compiler_Options_io.cpp
 ../src/joedb/compiler/c_wrapper.cpp
 ../src/joedb/compiler/nested_namespace.cpp
)

joedb_add_executable(joedb_embed
 ../src/joedb/compiler/joedb_embed.cpp
 ../src/joedb/io/base64.cpp
 ../src/joedb/compiler/nested_namespace.cpp
)

joedb_add_executable(joedb_embedded_client
 ../src/joedb/io/joedb_embedded_client.cpp
)

if (NOT DEFINED JOEDB_PORTABLE)
 joedb_add_executable(joedb_local_client
  ../src/joedb/io/joedb_local_client.cpp
 )
endif()

if (HAS_NETWORKING)
 joedb_add_executable(joedb_server ../src/joedb/io/joedb_server.cpp)

 if (libssh_FOUND)
  joedb_add_executable(joedb_multi_server
   ../src/joedb/io/joedb_multi_server.cpp
   ../src/joedb/db/multi_server.cpp
   ../src/joedb/db/multi_server_interpreted.cpp
   ../src/joedb/db/multi_server_custom_functions.cpp
  )
 endif()

 joedb_add_executable(joedb_network_client
  ../src/joedb/io/joedb_network_client.cpp
 )

 if(${CMAKE_SYSTEM_NAME} EQUAL CYGWIN)
  foreach(target
   joedb
   joedb_server
   joedb_test
   joedb_network_client
  )
   target_compile_definitions(${target} PRIVATE
    -D_WIN32_WINNT=0x0601 -D__USE_W32_SOCKETS
   )
  endforeach()
 endif()

endif()

if (libssh_FOUND)
 joedb_add_executable(joedb_ssh_client
  ../src/joedb/io/joedb_ssh_client.cpp
 )
endif()

#############################################################################
# Custom commands
#############################################################################
function(joedbc_build source_dir namespace)
 add_custom_command(
  OUTPUT
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}_interpreted.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}_interpreted.h
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}_readonly.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}_readonly.h
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.h
  COMMAND joedbc ${namespace}.joedbi ${namespace}.joedbc
  DEPENDS
   joedbc
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.joedbi
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.joedbc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}
 )
endfunction(joedbc_build)

joedbc_build(../test/compiler/db test)
joedbc_build(../test/compiler/db schema_v1)
joedbc_build(../test/compiler/db schema_v2)
joedbc_build(../test/compiler/db vector_test)
joedbc_build(../test/compiler/db multi_index)
joedbc_build(../test/compiler/db empty)
joedbc_build(../benchmark benchmarkdb)
joedbc_build(../src/joedb/db multi_server)

function(joedbc_build_wrapper source_dir namespace)
 add_custom_command(
  OUTPUT
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}_wrapper.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}_wrapper.h
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.h
  COMMAND joedbc ${namespace}.joedbi ${namespace}.joedbc
  DEPENDS
   joedbc
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.joedbi
   ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/${namespace}.joedbc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}
 )
endfunction(joedbc_build_wrapper)

joedbc_build_wrapper(../doc/source/tutorial tutorial)

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
 COMMAND joedbi test.joedb < db/test.joedbi
 DEPENDS
  joedbi
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/test.joedbi
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

#############################################################################
# joedb_embed test
#############################################################################
foreach(mode base64 escape)
 add_custom_command(
  OUTPUT
   ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/test_dbname_${mode}.h
   ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/test_dbname_${mode}.cpp
  COMMAND joedb_embed --${mode} ../test.joedb my_namespace::is_nested::test dbname_${mode}
  DEPENDS
   joedb_embed
   ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedb
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db
 )
endforeach(mode)

#############################################################################
# Targets below require databases to be compiled
#############################################################################
joedb_add_executable(tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/tutorial_main.cpp
)

joedb_add_executable(local_concurrency
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/local_concurrency.cpp
)

joedb_add_executable(concurrency_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/concurrency_tutorial.cpp
)

joedb_add_executable(step_by_step_replay
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/step_by_step_replay.cpp
)

joedb_add_executable(file_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/file_tutorial.cpp
)

joedb_add_executable(index_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ../doc/source/tutorial/index_tutorial.cpp
)

joedb_add_executable(wrapper_tutorial
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial_wrapper.cpp
 ../doc/source/tutorial/tutorial_main.c
)

joedb_add_executable(generate_translation_header
 ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/db/test.cpp
 ../test/compiler/generate_translation_header.cpp
)

joedb_add_executable(joedbc_insert
 ${CMAKE_CURRENT_SOURCE_DIR}/../benchmark/benchmarkdb.cpp
 ../benchmark/joedbc_insert.cpp
)

#############################################################################
# Packaging
#############################################################################
install(TARGETS joedbc RUNTIME DESTINATION bin)
install(TARGETS joedbi RUNTIME DESTINATION bin)
install(TARGETS joedb_convert RUNTIME DESTINATION bin)
install(TARGETS joedb_logdump RUNTIME DESTINATION bin)
install(TARGETS joedb_pack RUNTIME DESTINATION bin)
install(TARGETS joedb_to_json RUNTIME DESTINATION bin)
install(TARGETS joedb_merge RUNTIME DESTINATION bin)
install(TARGETS joedb_embed RUNTIME DESTINATION bin)
install(TARGETS joedb_embedded_client RUNTIME DESTINATION bin)
if (NOT JOEDB_PORTABLE)
 install(TARGETS joedb_local_client RUNTIME DESTINATION bin)
endif()
if(libssh_FOUND)
 install(TARGETS joedb_ssh_client RUNTIME DESTINATION bin)
endif()
if (HAS_NETWORKING)
 install(TARGETS joedb_network_client RUNTIME DESTINATION bin)
 install(TARGETS joedb_server RUNTIME DESTINATION bin)
 if (libssh_FOUND)
  install(TARGETS joedb_multi_server RUNTIME DESTINATION bin)
 endif()
endif()
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/../src/joedb/io/joedb_browser DESTINATION bin)
install(TARGETS joedb
 ARCHIVE DESTINATION lib
 LIBRARY DESTINATION lib
 RUNTIME DESTINATION bin
)

file(GLOB_RECURSE all_includes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/../src/ ../src/joedb/*.h)
foreach(file ${all_includes})
 get_filename_component(dir ${file} PATH)
 install(FILES ../src/${file} DESTINATION include/${dir})
endforeach()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
 execute_process(
  COMMAND lsb_release -s -i
  OUTPUT_VARIABLE CSN_SYSTEM
  OUTPUT_STRIP_TRAILING_WHITESPACE
 )
 execute_process(
  COMMAND lsb_release -s -r
  OUTPUT_VARIABLE CSN_RELEASE
  OUTPUT_STRIP_TRAILING_WHITESPACE
 )
 execute_process(
  COMMAND dpkg --print-architecture
  OUTPUT_VARIABLE CSN_ARCHI
  OUTPUT_STRIP_TRAILING_WHITESPACE
 )
 set(CPACK_SYSTEM_NAME "${CSN_SYSTEM}-${CSN_RELEASE}-${CSN_ARCHI}")
endif()
message("-- CPACK_SYSTEM_NAME = ${CPACK_SYSTEM_NAME}")

set(CPACK_PACKAGE_NAME "joedb")
set(CPACK_PACKAGE_VENDOR "Kayufu")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Journal-Only Embedded Database")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.remi-coulom.fr/joedb/")
set(CPACK_PACKAGE_CONTACT "Rémi Coulom")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.remi-coulom.fr/joedb/")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libssh-dev")
set(CPACK_DEBIAN_COMPRESSION_TYPE xz)

include(CPack)
