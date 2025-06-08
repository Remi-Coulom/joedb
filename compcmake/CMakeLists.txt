cmake_minimum_required(VERSION 3.24)

include("${CMAKE_CURRENT_LIST_DIR}/joedb_version.cmake")

project(joedb VERSION ${JOEDB_VERSION})

message("== JOEDB_VERSION = ${JOEDB_VERSION} ${JOEDB_VERSION_INT}")
message("== CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message("== CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # for YouCompleteMe
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
add_test(NAME build_all
 COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --config "$<CONFIG>"
)

include("${CMAKE_CURRENT_LIST_DIR}/compiler_flags.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/joedbc.cmake")

#############################################################################
# Dependencies
#############################################################################

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# GTest
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FetchContent_Declare(GTest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
  FIND_PACKAGE_ARGS NAMES GTest
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(GTest)
set(GTest_FOUND TRUE)
message("-- GTest_FOUND = ${GTest_FOUND}")

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
find_package(SQLite3)
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (SQLite3_FOUND)
 message("-- sqlite3 found")
 ipo_add_executable(sqlite3_insert ../benchmark/sqlite3_insert.cpp)
 target_link_libraries(sqlite3_insert ${SQLite3_LIBRARIES})
 target_include_directories(sqlite3_insert PRIVATE ${SQLite3_INCLUDE_DIRS})
else()
 message("## no sqlite3")
endif()

#############################################################################
# Includes
#############################################################################
include_directories(../test/compiler)

#############################################################################
# Build Targets
#############################################################################
joedb_add_executable(joedbi ../src/joedb/ui/joedbi.cpp)
joedb_add_executable(joedb_logdump ../src/joedb/ui/joedb_logdump.cpp)
joedb_add_executable(joedb_pack ../src/joedb/ui/joedb_pack.cpp)
joedb_add_executable(joedb_merge ../src/joedb/ui/joedb_merge.cpp)
joedb_add_executable(joedb_to_json ../src/joedb/ui/joedb_to_json.cpp)
joedb_add_executable(file_benchmark ../benchmark/file_benchmark.cpp)
joedb_add_executable(write_lock ../src/joedb/ui/write_lock.cpp)
joedb_add_executable(joedb_sha256sum ../src/joedb/ui/joedb_sha256sum.cpp)
joedb_add_executable(joedb_embed ../src/joedb/compiler/joedb_embed.cpp)
joedb_add_executable(joedb_client ../src/joedb/ui/joedb_client.cpp)
joedb_add_executable(joedb_push ../src/joedb/ui/joedb_push.cpp)

ipo_add_executable(joedb_convert
 ../src/joedb/Writable.cpp
 ../src/joedb/error/assert.cpp
 ../src/joedb/error/Destructor_Logger.cpp
 ../src/joedb/journal/File.cpp
 ../src/joedb/journal/Journal_Construction_Lock.cpp
 ../src/joedb/journal/Stream_File.cpp
 ../src/joedb/journal/Async_Reader.cpp
 ../src/joedb/journal/Buffered_File.cpp
 ../src/joedb/journal/Memory_File.cpp
 ../src/joedb/journal/Readonly_Journal.cpp
 ../src/joedb/journal/Writable_Journal.cpp
 ../src/joedb/ui/Arguments.cpp
 ../src/joedb/ui/joedb_convert.cpp
 ../src/joedb/ui/main_wrapper.cpp
 ../src/joedb/ui/process_journal_pair.cpp
)

if(GTest_FOUND)

 set(GTest_LIBRARIES GTest::gtest GTest::gtest_main)

 add_executable(joedb_file_test
  ../test/Test_Main.cpp
  ../src/joedb/error/assert.cpp
  ../src/joedb/error/String_Logger.cpp
  ../src/joedb/error/Destructor_Logger.cpp

  ../test/File_Test.cpp
  ../test/File_View_Test.cpp
  ../test/Upgradable_File_Test.cpp
  ../src/joedb/journal/Buffered_File.cpp
  ../src/joedb/journal/Async_Reader.cpp
  ../src/joedb/journal/Journal_Construction_Lock.cpp
  ../src/joedb/journal/Stream_File.cpp
  ../src/joedb/journal/File.cpp
  ../src/joedb/journal/Memory_File.cpp

  ../test/Buffered_File_Test.cpp

  ../test/Journal_Test.cpp
  ../src/joedb/journal/Readonly_Journal.cpp
  ../src/joedb/journal/Writable_Journal.cpp
  ../src/joedb/Readable.cpp
  ../src/joedb/Writable.cpp
  ../src/joedb/interpreted/Database.cpp
  ../src/joedb/interpreted/Database_Schema.cpp
  ../src/joedb/interpreted/Table.cpp
  ../src/joedb/is_identifier.cpp

  ../test/Buffer_Test.cpp
  ../test/Stream_File_Test.cpp
 )

 target_link_libraries(joedb_file_test ${GTest_LIBRARIES})

 if (NOT JOEDB_PORTABLE)
  target_sources(joedb_file_test PRIVATE ../src/joedb/journal/Portable_File.cpp)
 endif()

 joedb_add_executable(joedb_test
  ../doc/source/tutorial/src/settings/writable.cpp
  ../test/Arguments_Test.cpp
  ../test/Async_Test.cpp
  ../test/Buffer_Test.cpp
  ../test/endianness_Test.cpp
  ../test/diagnostics_Test.cpp
  ../test/Encoded_File_Test.cpp
  ../test/File_Test.cpp
  ../test/File_View_Test.cpp
  ../test/Upgradable_File_Test.cpp
  ../test/Freedom_Keeper_Test.cpp
  ../test/Buffered_File_Test.cpp
  ../test/Interpreted_Test.cpp
  ../test/Interpreter_Test.cpp
  ../test/is_identifier_Test.cpp
  ../test/Writable_Journal_Test.cpp
  ../test/Merge_Test.cpp
  ../test/Multiplexer_Test.cpp
  ../test/nested_namespace_Test.cpp
  ../test/pack_Test.cpp
  ../test/Polymorphic_File_Test.cpp
  ../test/get_time_string_Test.cpp
  ../test/Selective_Writable_Test.cpp
  ../test/SHA_256_Test.cpp
  ../test/Signal_Test.cpp
  ../test/ssh_Test.cpp
  ../test/Client_Test.cpp
  ../test/Stream_File_Test.cpp
  ../test/StringIO_Test.cpp
  ../test/Test_Main.cpp
  ../test/type_io_Test.cpp
  ../test/Journal_Test.cpp
  ../test/Server_Connection_Test.cpp
  ../test/Span_Test.cpp
  ../test/Local_Connection_Test.cpp
  ../test/File_Slice_Test.cpp
  ../test/File_Parser_Test.cpp
  ../test/Writable_Persistence_Test.cpp
  ../test/ui_Test.cpp
  ../test/compiler/Compiler_GTest.cpp
  ../test/compiler/embedded_GTest.cpp
  ../test/compiler/Interpreted_Database.cpp
  ../test/compiler/Readable_Test.cpp
  ../test/compiler/Readonly_Client.cpp
  ../test/compiler/single_row.cpp
  ../test/compiler/translation.h
  ../test/compiler/unique_index.cpp
  ../test/compiler/db/blob/writable.cpp
  ../test/compiler/db/empty/writable.cpp
  ../test/compiler/db/multi_index/writable.cpp
  ../test/compiler/db/schema_v1/writable.cpp
  ../test/compiler/db/schema_v2/writable.cpp
  ../test/compiler/db/test/writable.cpp
  ../test/compiler/db/vector_test/writable.cpp
  ../test/compiler/db/test_dbname_base64.cpp
  ../test/compiler/db/test_dbname_ascii.cpp
  ../src/joedb/ui/base64_decode.cpp
  ../src/joedb/journal/Portable_File.cpp
  ../src/joedb/error/String_Logger.cpp
 )

 target_link_libraries(joedb_test ${GTest_LIBRARIES})

 if (asio_FOUND)
  target_sources(joedb_test PRIVATE ../test/Server_Test.cpp)
 endif()

 if (CURL_FOUND)
  target_sources(joedb_test PRIVATE ../test/CURL_File_Test.cpp)
 endif()

 if (NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
  target_precompile_headers(joedb_file_test PRIVATE ../test/precompiled.h)
  target_precompile_headers(joedb_test PRIVATE ../test/precompiled.h)
 endif()

 add_test(NAME joedb_test
  COMMAND joedb_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test
 )
 SET_TESTS_PROPERTIES(joedb_test PROPERTIES DEPENDS build_all)
endif()

joedb_add_executable(joedbi_insert ../benchmark/joedbi_insert.cpp)

if (asio_FOUND)
 joedb_add_executable(joedb_server ../src/joedb/ui/joedb_server.cpp)
 joedb_add_executable(joedb_multi_server ../src/joedb/ui/joedb_multi_server.cpp)
endif()

joedb_add_executable(atomic_read ../test/atomic_read.cpp)
joedb_add_executable(atomic_write ../test/atomic_write.cpp)

joedbc_build(../test/compiler/db empty)
joedbc_build(../test/compiler/db blob)
joedbc_build(../test/compiler/db multi_index)
joedbc_build(../test/compiler/db schema_v1)
joedbc_build(../test/compiler/db schema_v2)
joedbc_build(../test/compiler/db test)
joedbc_build(../test/compiler/db vector_test)
joedbc_build(../benchmark benchmarkdb)
joedbc_build(../doc/source/tutorial/src tutorial)
joedbc_build(../doc/source/tutorial/src settings)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/translation.h
 COMMAND joedb_generate_translation_header > translation.h
 DEPENDS
  joedb_generate_translation_header
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedb
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler
)

add_custom_command(
 OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/../test/compiler/test.joedb
 COMMAND ${CMAKE_COMMAND} -E remove test.joedb
 COMMAND joedbi --new test.joedb < db/test.joedbi
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
  COMMAND joedb_embed --mode ${mode} ../test.joedb my_namespace::is_nested::test dbname_${mode}
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
 ../doc/source/tutorial/src/tutorial_main.cpp
 ../doc/source/tutorial/src/tutorial/writable.cpp
)

add_executable(minimal_tutorial
 ../doc/source/tutorial/src/tutorial/writable.cpp
 ../doc/source/tutorial/src/tutorial_main.cpp
 ../src/joedb/compiler/minimal_runtime.cpp
 ../src/joedb/ui/Arguments.cpp
 ../src/joedb/ui/main_wrapper.cpp
)

joedb_add_executable(tutorial_interpreter
 ../doc/source/tutorial/src/tutorial_interpreter.cpp
 ../doc/source/tutorial/src/tutorial/writable.cpp
)

if (NOT JOEDB_PORTABLE)
 joedb_add_executable(local_concurrency
  ../doc/source/tutorial/src/tutorial/writable.cpp
  ../doc/source/tutorial/src/local_concurrency.cpp
 )
endif()

joedb_add_executable(file_tutorial
 ../doc/source/tutorial/src/file_tutorial.cpp
 ../doc/source/tutorial/src/tutorial/writable.cpp
)

joedb_add_executable(concurrency_tutorial
 ../doc/source/tutorial/src/concurrency_tutorial.cpp
 ../doc/source/tutorial/src/tutorial/writable.cpp
)

joedb_add_executable(index_tutorial
 ../doc/source/tutorial/src/index_tutorial.cpp
 ../doc/source/tutorial/src/tutorial/writable.cpp
)

joedb_add_executable(settings
 ../doc/source/tutorial/src/settings_main.cpp
 ../doc/source/tutorial/src/settings/writable.cpp
)

if (asio_FOUND)
 joedb_add_executable(write_server_blob
  ../doc/source/tutorial/src/write_server_blob.cpp
 )
 joedb_add_executable(server_file_tutorial
  ../doc/source/tutorial/src/server_file_tutorial.cpp
 )
endif()

if (libssh_FOUND)
 joedb_add_executable(keepalive_test
  ../doc/source/tutorial/src/keepalive_test.cpp
 )
endif()

joedb_add_executable(client_lock
 ../doc/source/tutorial/src/client_lock.cpp
 ../doc/source/tutorial/src/tutorial/writable.cpp
)

joedb_add_executable(step_by_step_replay
 ../doc/source/tutorial/src/tutorial/writable.cpp
 ../doc/source/tutorial/src/step_by_step_replay.cpp
)

joedb_add_executable(joedb_generate_translation_header
 ../test/compiler/db/test/writable.cpp
 ../test/compiler/generate_translation_header.cpp
)

joedb_add_executable(joedbc_insert
 ../benchmark/benchmarkdb/writable.cpp
 ../benchmark/joedbc_insert.cpp
)

joedb_add_executable(update_large_string
 ../test/compiler/db/schema_v1/writable.cpp
 ../test/compiler/update_large_string.cpp
)

joedb_add_executable(procedure
 ../doc/source/tutorial/src/procedure.cpp
 ../doc/source/tutorial/src/tutorial/procedures/population/writable.cpp
 ../doc/source/tutorial/src/tutorial/writable.cpp
)

#############################################################################
# Fuzzers
#############################################################################
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND asio_FOUND AND NOT ANDROID)
 message("-- clang, compiling fuzzers")

 function(set_fuzz_options target)
  target_compile_definitions(${target} PRIVATE JOEDB_FUZZING)
  target_compile_options(${target} PRIVATE -fsanitize=address,fuzzer)
  target_link_options(${target} PRIVATE -fsanitize=address,fuzzer)
  target_link_libraries(${target} ${JOEDB_EXTERNAL_LIBS})
  add_dependencies(${target} all_joedbc)
 endfunction()

 add_library(joedb_fuzz STATIC
  ../src/external/wide_char_display_width.cpp
  ../src/joedb/interpreted/Database.cpp
  ../src/joedb/interpreted/Database_Schema.cpp
  ../src/joedb/is_identifier.cpp
  ../src/joedb/Multiplexer.cpp
  ../src/joedb/Readable.cpp
  ../src/joedb/Selective_Writable.cpp
  ../src/joedb/Writable.cpp
  ../src/joedb/concurrency/Channel.cpp
  ../src/joedb/concurrency/Client.cpp
  ../src/joedb/concurrency/Connection.cpp
  ../src/joedb/concurrency/Local_Channel.cpp
  ../src/joedb/concurrency/Server.cpp
  ../src/joedb/error/assert.cpp
  ../src/joedb/error/Destructor_Logger.cpp
  ../src/joedb/interpreted/Table.cpp
  ../src/joedb/ui/Blob_Reader_Command_Processor.cpp
  ../src/joedb/ui/Command_Interpreter.cpp
  ../src/joedb/ui/Command_Processor.cpp
  ../src/joedb/ui/Data_Manipulation_Command_Processor.cpp
  ../src/joedb/ui/Readable_Command_Processor.cpp
  ../src/joedb/ui/Writable_Command_Processor.cpp
  ../src/joedb/ui/Readable_Writable_Command_Processor.cpp
  ../src/joedb/ui/type_io.cpp
  ../src/joedb/ui/write_value.cpp
  ../src/joedb/ui/diagnostics.cpp
  ../src/joedb/ui/dump.cpp
  ../src/joedb/ui/json.cpp
  ../src/joedb/ui/base64.cpp
  ../src/joedb/ui/SQL_Dump_Writable.cpp
  ../src/joedb/ui/Interpreter_Dump_Writable.cpp
  ../src/joedb/ui/get_time_string.cpp
  ../src/joedb/ui/Progress_Bar.cpp
  ../src/joedb/journal/Async_Reader.cpp
  ../src/joedb/journal/Interpreted_File.cpp
  ../src/joedb/journal/Journal_Construction_Lock.cpp
  ../src/joedb/journal/Memory_File.cpp
  ../src/joedb/journal/File.cpp
  ../src/joedb/journal/Buffered_File.cpp
  ../src/joedb/journal/Writable_Journal.cpp
  ../src/joedb/journal/Stream_File.cpp
  ../src/joedb/journal/Readonly_Journal.cpp
  ../src/joedb/journal/Readonly_Interpreted_File.cpp
  ../src/joedb/journal/File_Hasher.cpp
 )

 set_fuzz_options(joedb_fuzz)

 function(add_fuzzer fuzzer)
  add_executable(${fuzzer}
   ../test/fuzz/${fuzzer}.cpp
  )
  set_fuzz_options(${fuzzer})
  target_link_libraries(${fuzzer} joedb_fuzz)
 endfunction()

 add_fuzzer(binary_journal_fuzzer)
 add_fuzzer(joedbc_fuzzer)
 add_fuzzer(joedbi_fuzzer)
 add_fuzzer(server_fuzzer)
endif()

#############################################################################
# Packaging
#############################################################################
install(TARGETS joedbc RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedbi RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedb_logdump RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedb_pack RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedb_to_json RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedb_merge RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedb_embed RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedb_generate_translation_header RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedb_client RUNTIME DESTINATION bin COMPONENT bin)
install(TARGETS joedb_push RUNTIME DESTINATION bin COMPONENT bin)
if (asio_FOUND)
 install(TARGETS joedb_server RUNTIME DESTINATION bin COMPONENT bin)
 install(TARGETS joedb_multi_server RUNTIME DESTINATION bin COMPONENT bin)
endif()
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/joedb_browser DESTINATION bin COMPONENT bin)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/joedb_edit DESTINATION bin COMPONENT bin)
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/joedb_inplace_pack DESTINATION bin COMPONENT bin)
install(TARGETS joedb
 ARCHIVE DESTINATION lib COMPONENT bin
 LIBRARY DESTINATION lib COMPONENT bin
 RUNTIME DESTINATION bin COMPONENT bin
)

file(GLOB_RECURSE all_includes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/../src/ ../src/joedb/*.h)
foreach(file ${all_includes})
 get_filename_component(dir ${file} PATH)
 install(FILES ../src/${file} DESTINATION include/${dir} COMPONENT dev)
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
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.joedb.org/")
set(CPACK_PACKAGE_CONTACT "Rémi Coulom")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_PACKAGE_SECTION "database")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.joedb.org/")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "libssh-dev, libbrotli-dev, libcurl4-openssl-dev")
set(CPACK_DEBIAN_BIN_PACKAGE_DEPENDS "libssh-4, libbrotli1, libcurl4")
set(CPACK_DEBIAN_COMPRESSION_TYPE xz)

include(CPack)
