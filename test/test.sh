#!/bin/bash
build_dir=../compcmake/gcc_debug
cmake --build $build_dir --target joedb_test || exit 1
rm -vf *.tmp
$build_dir/joedb_test --gtest_filter="$1*"
