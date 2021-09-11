#!/bin/bash
set -e

# sudo apt-get install libfuzzer-8-dev

#############################################################################
build()
#############################################################################
{
 echo "Building $1..."
 echo "run with: ./$1_fuzzer $1_corpus"

 clang++\
  -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION\
  -fsanitize-coverage=trace-pc-guard\
  -fsanitize=address\
  -std=c++11\
  -I ../../src\
  "$1"_fuzzer.cpp\
  ../../src/external/wide_char_display_width.cpp\
  ../../src/joedb/interpreter/Database.cpp\
  ../../src/joedb/interpreter/Database_Schema.cpp\
  ../../src/joedb/is_identifier.cpp\
  ../../src/joedb/Readable.cpp\
  ../../src/joedb/Writable.cpp\
  ../../src/joedb/Multiplexer.cpp\
  ../../src/joedb/interpreter/Table.cpp\
  ../../src/joedb/Destructor_Logger.cpp\
  ../../src/joedb/io/Interpreter.cpp\
  ../../src/joedb/io/type_io.cpp\
  ../../src/joedb/io/dump.cpp\
  ../../src/joedb/io/json.cpp\
  ../../src/joedb/io/base64.cpp\
  ../../src/joedb/io/SQL_Dump_Writable.cpp\
  ../../src/joedb/io/Interpreter_Dump_Writable.cpp\
  ../../src/joedb/io/get_time_string.cpp\
  ../../src/joedb/journal/diagnostics.cpp\
  ../../src/joedb/journal/File.cpp\
  ../../src/joedb/journal/Generic_File.cpp\
  ../../src/joedb/journal/Writable_Journal.cpp\
  ../../src/joedb/journal/SHA_256.cpp\
  ../../src/joedb/journal/Stream_File.cpp\
  ../../src/joedb/journal/Readonly_Journal.cpp\
  /usr/lib/clang/8/lib/linux/libclang_rt.fuzzer-x86_64.a\
  -o "$1"_fuzzer
}

build "binary_journal"
build "joedbi"

cd ../../compcmake/ninja_clang
ninja joedb_test
cd -
build "joedbc"
