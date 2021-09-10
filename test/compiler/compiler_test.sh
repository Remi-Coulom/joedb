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
