#!/bin/bash
set -e

cd ../src
src_dir=`pwd`
cd -

mkdir -p html

lcov --directory ../compcmake/gcc_coverage\
     --capture\
     --ignore-errors mismatch\
     --rc lcov_branch_coverage=1\
     --output-file all.info || exit 1

lcov --remove all.info '/usr/*' '*/joedb/test*' '*joedb/src/joedb/db/*' '*/joedb/doc/source/tutorial/*'\
     --rc lcov_branch_coverage=1\
     --output app.info || exit 1

genhtml --prefix "$src_dir"\
        --demangle-cpp\
        --rc genhtml_branch_coverage=1\
        --output-directory html app.info || exit 1

rm -vf *.info

echo xdg-open html/index.html
