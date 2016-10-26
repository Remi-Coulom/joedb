#!/bin/bash
set -e

dir=../compcmake/ninja_release

cd $dir
ninja
cd -

rm -vf *.joedb
$dir/joedbc joedbc_insert.joedbi joedbc_insert.joedbc

cd $dir
ninja joedbc_insert
cd -

time $dir/joedbc_insert $*
