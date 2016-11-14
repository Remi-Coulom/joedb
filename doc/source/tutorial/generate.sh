#!/bin/bash

dir=../../../compcmake/ninja_debug

cd $dir
ninja || exit 1
cd -

rm -vf tutorial.joedb
$dir/joedbc tutorial.joedbi tutorial.joedbc || exit 1

cd $dir
ninja tutorial index_tutorial || exit 1
cd -

$dir/index_tutorial >index_tutorial.out
$dir/tutorial >tutorial.out
cat tutorial.out

$dir/joedb_logdump tutorial.joedb >logdump.out
$dir/joedb_logdump --sql tutorial.joedb >logdump.sql
echo help | $dir/joedbi >joedbi_help.out
