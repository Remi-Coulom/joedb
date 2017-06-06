#!/bin/bash

dir=../../../compcmake/ninja_debug

cd $dir
ninja tutorial index_tutorial wrapper_tutorial || exit 1
cd -

rm -vf tutorial.joedb index_tutorial.joedb

$dir/index_tutorial >index_tutorial.out
$dir/tutorial >tutorial.out
cat tutorial.out

$dir/joedb_logdump tutorial.joedb >logdump.out
$dir/joedb_logdump --sql tutorial.joedb >logdump.sql
$dir/joedb_to_json tutorial.joedb >tutorial.json
echo help | $dir/joedbi >joedbi_help.out

sed -e s#VERSION#`echo $(<../../../VERSION) | tr -d '"'`# compiling-template.sh >compiling.sh
