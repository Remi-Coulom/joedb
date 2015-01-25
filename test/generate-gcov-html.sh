#!/bin/bash

cd ../src
src_dir=`pwd`
cd -

mkdir -p html

lcov --directory ../compcmake/ninja_debug\
     --capture\
     --rc lcov_branch_coverage=1\
     --output-file all.info || exit 1

lcov --remove all.info '/usr/*' '*_Test.cpp' '*Test_Main.cpp' '*/gtest-*/*'\
     --rc lcov_branch_coverage=1\
     --output app.info || exit 1

genhtml --prefix "$src_dir"\
        --demangle-cpp\
        --rc genhtml_branch_coverage=1\
        --output-directory html app.info || exit 1

rm -vf *.info

echo xdg-open html/index.html
