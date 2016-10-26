#!/bin/bash
build_dir=../../compcmake/ninja_debug

cd "$build_dir"
ninja
cd -

rm -rvf *.joedb
"$build_dir"/joedbi test.joedb <test.joedbi
"$build_dir"/joedbc test.joedbi test.joedbc
"$build_dir"/joedbc schema_v1.joedbi schema_v1.joedbc
"$build_dir"/joedbc schema_v2.joedbi schema_v2.joedbc


cd "$build_dir"
ninja generate_translation_header
cd -
"$build_dir"/generate_translation_header >translation.h

cd "$build_dir"
ninja compiler_test
cd -
"$build_dir"/compiler_test >compiler_test.out

#
# Check differences with reference output
#
if diff compiler_test.out compiler_test.reference; then
 echo "Everything is OK";
else
 echo "Error: difference detected with reference output";
fi
