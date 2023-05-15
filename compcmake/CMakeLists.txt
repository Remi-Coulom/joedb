if (UNIX)
 cmake_minimum_required(VERSION 3.2)
else()
 #For some reasons requiring 3.9 is necessary in MSVC
 cmake_minimum_required(VERSION 3.9)
endif()

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

include("${CMAKE_CURRENT_LIST_DIR}/compiler_flags.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/joedbc.cmake")

#############################################################################
# Dependencies
#############################################################################

# Google Test
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(GOOGLE_TEST
 ${CMAKE_CURRENT_SOURCE_DIR}/../submodules/googletest/googletest
)

# SQLite 3
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(SQLite3)
if (SQLite3_FOUND)
 ipo_add_executable(sqlite3_insert ../benchmark/sqlite3_insert.cpp)
 target_link_libraries(sqlite3_insert ${SQLite3_LIBRARIES})
 target_include_directories(sqlite3_insert PRIVATE ${SQLite3_INCLUDE_DIRS})
else()
 message("== WARNING: no sqlite3: Try sudo apt install libsqlite3-dev or vcpkg install sqlite3:x64-windows")
endif()

#############################################################################
# Includes
#############################################################################
include_directories(
 ../test/compiler
 ${GOOGLE_TEST}
 ${GOOGLE_TEST}/include
)

#############################################################################
# Build Targets
#############################################################################
joedb_add_executable(joedbi ../src/joedb/io/joedbi.cpp)
joedb_add_executable(joedb_logdump ../src/joedb/io/joedb_logdump.cpp)
joedb_add_executable(joedb_pack ../src/joedb/io/joedb_pack.cpp)
joedb_add_executable(joedb_merge ../src/joedb/io/joedb_merge.cpp)
joedb_add_executable(joedb_to_json ../src/joedb/io/joedb_to_json.cpp)
joedb_add_executable(file_benchmark ../benchmark/file_benchmark.cpp)
joedb_add_executable(write_lock ../src/joedb/io/write_lock.cpp)
joedb_add_executable(joedb_sha256sum ../src/joedb/io/joedb_sha256sum.cpp)
joedb_add_executable(joedb_embed ../src/joedb/compiler/joedb_embed.cpp)

ipo_add_executable(joedb_convert
 ../src/joedb/io/joedb_convert.cpp
 ../src/joedb/Destructor_Logger.cpp
 ../src/joedb/Blob.cpp
 ../src/joedb/Writable.cpp
 ../src/joedb/journal/File.cpp
 ../src/joedb/journal/Stream_File.cpp
 ../src/joedb/journal/Generic_File.cpp
 ../src/joedb/journal/Readonly_Journal.cpp
 ../src/joedb/journal/SHA_256.cpp
 ../src/joedb/journal/Writable_Journal.cpp
)

if(EXISTS "${GOOGLE_TEST}/include")
 joedb_add_executable(joedb_test
  ../test/endianness_Test.cpp
  ../test/diagnostics_Test.cpp
  ../test/File_Test.cpp
  ../test/Generic_File_Test.cpp
  ../test/Freedom_Keeper_Test.cpp
  ../test/Interpreted_Test.cpp
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
  ../test/compiler/Blob.cpp
  ../test/compiler/Compiler_GTest.cpp
  ../test/compiler/embedded_GTest.cpp
  ../test/compiler/db/blob.cpp
  ../test/compiler/db/test.cpp
  ../test/compiler/db/multi_index.cpp
  ../test/compiler/db/schema_v1.cpp
  ../test/compiler/db/schema_v2.cpp
  ../test/compiler/db/vector_test.cpp
  ../test/compiler/db/empty.cpp
  ../test/compiler/translation.h
  ../test/compiler/db/test_dbname_base64.cpp
  ../test/compiler/db/test_dbname_ascii.cpp
  ../src/joedb/io/base64_decode.cpp
  ../src/joedb/journal/Portable_File.cpp
  ../src/joedb/String_Logger.cpp
 )

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
  )
 endif()

 joedb_add_executable(joedb_network_client
  ../src/joedb/io/joedb_network_client.cpp
 )
endif()

if (libssh_FOUND)
 joedb_add_executable(joedb_ssh_client
  ../src/joedb/io/joedb_ssh_client.cpp
 )
endif()

joedbc_build(../test/compiler/db test)
joedbc_build(../test/compiler/db schema_v1)
joedbc_build(../test/compiler/db schema_v2)
joedbc_build(../test/compiler/db vector_test)
joedbc_build(../test/compiler/db multi_index)
joedbc_build(../test/compiler/db empty)
joedbc_build(../test/compiler/db blob)
joedbc_build(../benchmark benchmarkdb)
joedbc_build(../src/joedb/db multi_server)
joedbc_build(../doc/source/tutorial tutorial)

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
foreach(mode base64 ascii)
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

if (NOT JOEDB_PORTABLE)
 joedb_add_executable(local_concurrency
  ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
  ../doc/source/tutorial/local_concurrency.cpp
 )
endif()

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

#joedb_add_executable(wrapper_tutorial
# ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial.cpp
# ${CMAKE_CURRENT_SOURCE_DIR}/../doc/source/tutorial/tutorial_wrapper.cpp
# ../doc/source/tutorial/tutorial_main.c
#)

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
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/joedb_browser DESTINATION bin)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/joedb_inplace_pack DESTINATION bin)
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
