#!/bin/bash
TZ='Europe/Paris'
cd ../compcmake/ninja_debug
ninja joedb_test || exit 1
cd -
git clean -f -d -x
../compcmake/ninja_debug/joedb_test --gtest_filter="$1*"
