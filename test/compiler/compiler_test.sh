#!/bin/bash
set -e
build_dir=../../compcmake/ninja_debug

rm -rvf *.joedb compiler_test.out

cd "$build_dir"
ninja joedbi joedbc compiler_test
cd -

"$build_dir"/joedbi vector_hole.joedb <vector_hole.joedbi
"$build_dir"/joedbi vector_delete.joedb <vector_delete.joedbi
"$build_dir"/joedbi multi_index.joedb <multi_index.joedbi

"$build_dir"/compiler_test >compiler_test.out

#
# Check differences with reference output
#
if diff compiler_test.reference compiler_test.out; then
 echo "Everything is OK";
else
 echo "Error: difference detected with reference output";
fi
