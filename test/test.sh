#!/bin/bash
TZ='Europe/Paris'

if [ -d ../compcmake/ninja_debug ]; then
 build_dir=../compcmake/ninja_debug
 build_command=ninja
else
 build_dir=../compcmake/debug
 build_command=make
fi

cd $build_dir
$build_command joedb_test || exit 1
cd -
$build_dir/joedb_test --gtest_filter="$1*"
