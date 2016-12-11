#!/bin/bash

# http://llvm.org/docs/LibFuzzer.html#versions
if [ ! -d third_party ]; then
 echo Installing clang ...
 mkdir TMP_CLANG
 cd TMP_CLANG
 git clone https://chromium.googlesource.com/chromium/src/tools/clang
 cd ..
 TMP_CLANG/clang/scripts/update.py
fi

# http://llvm.org/docs/LibFuzzer.html#building
if [ ! -f libFuzzer.a ]; then
 echo Building libFuzzer.a ...
 git clone https://chromium.googlesource.com/chromium/llvm-project/llvm/lib/Fuzzer
 PATH=`pwd`/third_party/llvm-build/Release+Asserts/bin:$PATH
 ./Fuzzer/build.sh
fi

./third_party/llvm-build/Release+Asserts/bin/clang++\
 -fsanitize-coverage=trace-pc-guard\
 -fsanitize=address\
 -std=c++11\
 -I ../../src/interpreter/abstract\
 -I ../../src/includes/joedb\
 fuzz_target.cpp\
 ../../src/interpreter/abstract/Database.cpp\
 ../../src/interpreter/abstract/Listener.cpp\
 ../../src/interpreter/abstract/Multiplexer.cpp\
 ../../src/interpreter/abstract/Safe_Listener.cpp\
 ../../src/interpreter/abstract/Table.cpp\
 ../../src/journal/Generic_File.cpp\
 ../../src/journal/Journal_File.cpp\
 ../../src/journal/Stream_File.cpp\
 ./libFuzzer.a\
 -o joedb_fuzz
