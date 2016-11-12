#!/bin/bash

dir=../../../compcmake/ninja_debug

cd $dir
ninja || exit 1
cd -

rm -vf tutorial.joedb
$dir/joedbc tutorial.joedbi tutorial.joedbc || exit 1

cd $dir
ninja tutorial || exit 1
cd -

$dir/tutorial >tutorial.out
cat tutorial.out

joedb_logdump tutorial.joedb >logdump.out
