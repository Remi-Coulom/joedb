#!/bin/bash

#############################################################################
# http://llvm.org/docs/LibFuzzer.html#versions
#############################################################################
if [ ! -d third_party ]; then
 echo Installing clang ...
 mkdir TMP_CLANG
 cd TMP_CLANG
 git clone https://chromium.googlesource.com/chromium/src/tools/clang
 cd ..
 TMP_CLANG/clang/scripts/update.py
fi

#############################################################################
# http://llvm.org/docs/LibFuzzer.html#building
#############################################################################
if [ ! -f libFuzzer.a ]; then
 echo Building libFuzzer.a ...
 git clone https://chromium.googlesource.com/chromium/llvm-project/llvm/lib/Fuzzer
 PATH=`pwd`/third_party/llvm-build/Release+Asserts/bin:$PATH
 ./Fuzzer/build.sh
fi

#############################################################################
build()
#############################################################################
{
 ./third_party/llvm-build/Release+Asserts/bin/clang++\
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
  ../../src/interpreter/abstract/Writeable.cpp\
  ../../src/interpreter/abstract/Multiplexer.cpp\
  ../../src/interpreter/abstract/Table.cpp\
  ../../src/interpreter/io/Interpreter.cpp\
  ../../src/interpreter/io/Dump_Writeable.cpp\
  ../../src/interpreter/io/type_io.cpp\
  ../../src/interpreter/io/dump.cpp\
  ../../src/interpreter/io/json.cpp\
  ../../src/interpreter/io/base64.cpp\
  ../../src/journal/diagnostics.cpp\
  ../../src/journal/File.cpp\
  ../../src/journal/Generic_File.cpp\
  ../../src/journal/Journal_File.cpp\
  ../../src/journal/Stream_File.cpp\
  ../../src/journal/Readonly_Journal.cpp\
  ./libFuzzer.a\
  -o "$1"_fuzzer
}

build "binary_journal"
build "joedbi"

cd ../compiler
./compiler_test.sh
cd -
build "joedbc"
