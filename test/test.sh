#!/bin/bash
set -e
cd ../doc/source/tutorial
./generate.sh
cd -
build_dir=../compcmake/gcc_debug
cmake --build $build_dir --target joedb_test
rm -vf *.tmp

if [ "$1" == "" ]; then
 filter='*'
else
 filter="$1"
fi

$build_dir/joedb_test --gtest_filter="$filter"
