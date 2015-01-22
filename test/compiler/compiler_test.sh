#!/bin/bash
build_dir=../../compcmake/ninja_debug

cd "$build_dir"
ninja
cd -

rm -rvf test.joedb
"$build_dir"/joedbi test.joedb <test.joedbi
"$build_dir"/joedbc test.joedb testdb >testdb.h
