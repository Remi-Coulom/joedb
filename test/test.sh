#!/bin/bash
set -e

if [ "$1" == "" ]; then
 cd ../doc/source/tutorial
 ./generate.sh
 cd -
fi

build_dir=../compcmake/gcc_asan
cmake --build $build_dir --target joedb_test
rm -vf *.tmp

if [ "$1" == "" ]; then
 filter='*'
else
 filter="$1"
fi

$build_dir/joedb_test --gtest_filter="$filter"
