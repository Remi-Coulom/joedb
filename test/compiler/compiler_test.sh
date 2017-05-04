#!/bin/bash
set -e
build_dir=../../compcmake/ninja_debug

cd "$build_dir"
ninja
cd -

rm -rvf *.joedb compiler_test.out
"$build_dir"/joedbi test.joedb <test.joedbi
"$build_dir"/joedbi vector_hole.joedb <vector_hole.joedbi
"$build_dir"/joedbi vector_delete.joedb <vector_delete.joedbi
"$build_dir"/joedbi multi_index.joedb <multi_index.joedbi
"$build_dir"/joedbc test.joedbi test.joedbc
"$build_dir"/joedbc schema_v1.joedbi schema_v1.joedbc
"$build_dir"/joedbc schema_v2.joedbi schema_v2.joedbc
"$build_dir"/joedbc vector_test.joedbi vector_test.joedbc
"$build_dir"/joedbc multi_index.joedbi multi_index.joedbc


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
if diff compiler_test.reference compiler_test.out; then
 echo "Everything is OK";
else
 echo "Error: difference detected with reference output";
fi
