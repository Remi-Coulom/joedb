#!/bin/bash
../../../third_party/llvm-build/Release+Asserts/bin/clang++\
 -fsanitize-coverage=trace-pc-guard\
 -fsanitize=address\
 -std=c++11\
 -I ../../src/interpreter/abstract\
 fuzz_target.cpp\
 ../../src/interpreter/abstract/Database.cpp\
 ../../src/interpreter/abstract/Listener.cpp\
 ../../src/interpreter/abstract/Table.cpp\
 ../../src/journal/Generic_File.cpp\
 ../../src/journal/Journal_File.cpp\
 ../../src/journal/Stream_File.cpp\
 ../../../Fuzzer/libFuzzer.a\
 -o joedb_fuzz
