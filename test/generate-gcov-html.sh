#!/bin/bash
set -e

cd ../src
src_dir=`pwd`
cd -

mkdir -p html

lcov --directory ../compcmake/gcc_coverage\
     --capture\
     --filter exception,orphan\
     --ignore-errors mismatch,inconsistent\
     --rc lcov_branch_coverage=1\
     --output-file all.info

lcov --remove all.info '/usr/*' '*/joedb/TYPE_MACRO.h' '*/joedb/test*' '*joedb/src/joedb/db/*' '*/joedb/doc/source/tutorial/*' 'certify/*'\
     --ignore-errors mismatch,inconsistent\
     --rc lcov_branch_coverage=1\
     --output app.info

genhtml --prefix "$src_dir"\
        --filter exception,orphan\
        --ignore-errors mismatch,inconsistent\
        --demangle-cpp\
        --rc genhtml_branch_coverage=1\
        --output-directory html app.info

rm -vf *.info

echo xdg-open html/index.html
echo explorer.exe html\\\\index.html
