#!/bin/bash
cd ../compcmake/ninja_debug
ninja joedb_test || exit 1
cd -
../compcmake/ninja_debug/joedb_test --gtest_filter="$1*"
