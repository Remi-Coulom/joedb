#!/bin/bash

set -e

if [ -d ../../compcmake/debug ]; then
 build_dir=../../compcmake/debug
 build_command=make
else
 build_dir=../../compcmake/ninja_debug
 build_command=ninja
fi

rm -rvf *.joedb compiler_test.out

cd "$build_dir"
$build_command joedbi joedbc compiler_test embedded_test
cd -

generate_db()
{
 echo "echo off" | cat - $1.joedbi | "$build_dir/joedbi" $1.joedb
}

generate_db vector_hole
generate_db vector_hole_by_vector_insert
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
