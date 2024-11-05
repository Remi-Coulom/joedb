#!/bin/bash
set -e

#############################################################################
build()
#############################################################################
{
 echo "Building $1..."
 echo "run with: ./$1_fuzzer $1_corpus"

 clang++\
  -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION\
  -fsanitize=address,fuzzer\
  -std=c++17\
  -O2\
  -I ../../src\
  "$1"_fuzzer.cpp\
  ../../src/external/wide_char_display_width.cpp\
  ../../src/joedb/Blob.cpp\
  ../../src/joedb/Destructor_Logger.cpp\
  ../../src/joedb/interpreter/Database.cpp\
  ../../src/joedb/interpreter/Database_Schema.cpp\
  ../../src/joedb/is_identifier.cpp\
  ../../src/joedb/Multiplexer.cpp\
  ../../src/joedb/Posthumous_Catcher.cpp\
  ../../src/joedb/Posthumous_Thrower.cpp\
  ../../src/joedb/Readable.cpp\
  ../../src/joedb/Selective_Writable.cpp\
  ../../src/joedb/Writable.cpp\
  ../../src/joedb/interpreter/Table.cpp\
  ../../src/joedb/io/Command_Interpreter.cpp\
  ../../src/joedb/io/Command_Processor.cpp\
  ../../src/joedb/io/Readable_Command_Processor.cpp\
  ../../src/joedb/io/Writable_Command_Processor.cpp\
  ../../src/joedb/io/Readable_Writable_Command_Processor.cpp\
  ../../src/joedb/io/type_io.cpp\
  ../../src/joedb/io/write_value.cpp\
  ../../src/joedb/io/dump.cpp\
  ../../src/joedb/io/json.cpp\
  ../../src/joedb/io/base64.cpp\
  ../../src/joedb/io/SQL_Dump_Writable.cpp\
  ../../src/joedb/io/Interpreter_Dump_Writable.cpp\
  ../../src/joedb/io/get_time_string.cpp\
  ../../src/joedb/journal/Abstract_File.cpp\
  ../../src/joedb/journal/diagnostics.cpp\
  ../../src/joedb/journal/Interpreted_File.cpp\
  ../../src/joedb/journal/Journal_Construction_Lock.cpp\
  ../../src/joedb/journal/Memory_File.cpp\
  ../../src/joedb/journal/File.cpp\
  ../../src/joedb/journal/Generic_File.cpp\
  ../../src/joedb/journal/Writable_Journal.cpp\
  ../../src/joedb/journal/Stream_File.cpp\
  ../../src/joedb/journal/Readonly_Journal.cpp\
  ../../src/joedb/journal/Readonly_Interpreted_File.cpp\
  -o "$1"_fuzzer
}

build "binary_journal"
build "joedbi"
build "joedbc"
