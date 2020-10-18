#!/bin/bash
set -e

# sudo apt-get install libfuzzer-8-dev

#############################################################################
build()
#############################################################################
{
 clang++-8\
  -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION\
  -fsanitize-coverage=trace-pc-guard\
  -fsanitize=address\
  -std=c++11\
  -I ../../src\
  "$1"_fuzzer.cpp\
  ../../src/joedb/external/wide_char_display_width.c\
  ../../src/joedb/interpreter/Database.cpp\
  ../../src/joedb/is_identifier.cpp\
  ../../src/joedb/Writable.cpp\
  ../../src/joedb/Multiplexer.cpp\
  ../../src/joedb/interpreter/Table.cpp\
  ../../src/joedb/io/Interpreter.cpp\
  ../../src/joedb/io/Dump_Writable.cpp\
  ../../src/joedb/io/type_io.cpp\
  ../../src/joedb/io/dump.cpp\
  ../../src/joedb/io/json.cpp\
  ../../src/joedb/io/base64.cpp\
  ../../src/joedb/journal/diagnostics.cpp\
  ../../src/joedb/journal/File.cpp\
  ../../src/joedb/journal/Generic_File.cpp\
  ../../src/joedb/journal/Writable_Journal.cpp\
  ../../src/joedb/journal/Stream_File.cpp\
  ../../src/joedb/journal/Readonly_Journal.cpp\
  /usr/lib/clang/8/lib/linux/libclang_rt.fuzzer-x86_64.a\
  -o "$1"_fuzzer
}

build "binary_journal"
build "joedbi"

cd ../compiler
./compiler_test.sh
cd -
build "joedbc"
