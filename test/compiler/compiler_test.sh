#!/bin/bash

set -e
build_dir=../../compcmake/ninja_debug

rm -rvf *.joedb compiler_test.out

cd "$build_dir"
ninja joedbi joedbc compiler_test embedded_test
cd -

generate_db()
{
 echo "echo off" | cat - $1.joedbi | "$build_dir/joedbi" $1.joedb
}

generate_db vector_hole
generate_db vector_delete
generate_db multi_index


"$build_dir"/compiler_test >compiler_test.out
"$build_dir"/embedded_test >>compiler_test.out

#
# Check differences with reference output
#
if diff compiler_test.reference compiler_test.out; then
 echo "Everything is OK";
else
 echo "Error: difference detected with reference output";
fi
