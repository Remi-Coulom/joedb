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
  -I ../../src/external\
  -I ../../src/interpreter/abstract\
  -I ../../src/interpreter/io\
  -I ../../src/includes/joedb\
  -I ../../src/includes\
  -I ../../src/journal\
  "$1"_fuzzer.cpp\
  ../../src/external/wide_char_display_width.c\
  ../../src/interpreter/abstract/Database.cpp\
  ../../src/interpreter/abstract/is_identifier.cpp\
  ../../src/interpreter/abstract/Writable.cpp\
  ../../src/interpreter/abstract/Multiplexer.cpp\
  ../../src/interpreter/abstract/Table.cpp\
  ../../src/interpreter/io/Interpreter.cpp\
  ../../src/interpreter/io/Dump_Writable.cpp\
  ../../src/interpreter/io/type_io.cpp\
  ../../src/interpreter/io/dump.cpp\
  ../../src/interpreter/io/json.cpp\
  ../../src/interpreter/io/base64.cpp\
  ../../src/journal/diagnostics.cpp\
  ../../src/journal/File.cpp\
  ../../src/journal/Generic_File.cpp\
  ../../src/journal/Writable_File.cpp\
  ../../src/journal/Stream_File.cpp\
  ../../src/journal/Readonly_Journal.cpp\
  /usr/lib/clang/8/lib/linux/libclang_rt.fuzzer-x86_64.a\
  -o "$1"_fuzzer
}

build "binary_journal"
build "joedbi"

cd ../compiler
./compiler_test.sh
cd -
build "joedbc"
