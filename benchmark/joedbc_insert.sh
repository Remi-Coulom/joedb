#!/bin/bash
dir=../compcmake/ninja_release

cd $dir
ninja
cd -

rm -vf insert.joedb benchmark.h
$dir/joedbi insert.joedb <joedbc_insert.joedbi
$dir/joedbc insert.joedb benchmark >benchmark.h

cd $dir
ninja joedbc_insert
cd -

time $dir/joedbc_insert $*
