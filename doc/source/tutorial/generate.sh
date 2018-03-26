#!/bin/bash

dir=../../../compcmake/ninja_debug

cd $dir
ninja tutorial index_tutorial wrapper_tutorial joedb_logdump joedb_to_json joedbi || exit 1
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

rm -rvf merge_1.joedb merge_2.joedb merged.joedb
joedbi merge_1.joedb <merge_1.joedbi
joedbi merge_2.joedb <merge_2.joedbi
joedb_merge merge_1.joedb merge_2.joedb merged.joedb
joedb_to_json merge_1.joedb >merge_1.json
joedb_to_json merge_2.joedb >merge_2.json
joedb_to_json merged.joedb >merged.json
joedb_merge 2>joedb_merge.out
