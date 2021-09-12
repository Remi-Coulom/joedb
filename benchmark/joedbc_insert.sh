#!/bin/bash
set -e

dir=../compcmake/gcc_release
cmake --build $dir --target joedbc_insert

rm -vf insert.joedb
time $dir/joedbc_insert $*
