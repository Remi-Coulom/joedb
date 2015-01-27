#!/bin/bash

dir=../../../compcmake/ninja_debug

cd $dir
ninja
cd -

rm -vf tutorial.joedb
$dir/joedbi tutorial.joedb <tutorial.joedbi
$dir/joedbc tutorial.joedb tutorial >tutorial.h

cd $dir
ninja tutorial
cd -

$dir/tutorial
