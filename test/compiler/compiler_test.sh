#!/bin/bash
set -e
build_dir=../../compcmake/ninja_debug

cd "$build_dir"
ninja
cd -

rm -rvf test.joedb
"$build_dir"/joedbi test.joedb <test.joedbi
"$build_dir"/joedbc test.joedbi test.joedbc >testdb.h

cd "$build_dir"
ninja compiler_test
cd -
"$build_dir"/compiler_test >compiler_test.out
diff compiler_test.out compiler_test.reference
