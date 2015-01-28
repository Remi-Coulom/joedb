#!/bin/bash

dir=../../../compcmake/ninja_debug

cd $dir
ninja || exit 1
cd -

rm -vf tutorial.joedb
$dir/joedbi tutorial.joedb <tutorial.joedbi || exit 1
$dir/joedbc tutorial.joedbi tutorial >tutorial.h || exit 1

cd $dir
ninja tutorial || exit 1
cd -

$dir/tutorial
