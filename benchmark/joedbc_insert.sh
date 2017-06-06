#!/bin/bash
set -e

dir=../compcmake/ninja_release

cd $dir
ninja joedbc_insert
cd -

rm -vf insert.joedb
time $dir/joedbc_insert $*
