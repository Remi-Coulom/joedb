#!/bin/bash

lcov --directory ../compcmake/ninja_debug\
     --capture\
     --rc lcov_branch_coverage=1\
     --output-file all.info || exit 1

lcov --remove all.info '/usr/*'\
     --rc lcov_branch_coverage=1\
     --output app.info || exit 1

mkdir -p html

cd ../src
src_dir=`pwd`
cd -

genhtml --prefix "$src_dir"\
        --demangle-cpp\
        --rc genhtml_branch_coverage=1\
        --output-directory html app.info || exit 1

echo xdg-open html/index.html
