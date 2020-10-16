#!/bin/bash
set -e

dir=../../../compcmake/ninja_debug

cd $dir
ninja
cd -

rm -f tutorial.joedb index_tutorial.joedb

$dir/index_tutorial >index_tutorial.out
$dir/tutorial >tutorial.out
$dir/concurrency_tutorial >concurrency_tutorial.out

$dir/joedb_logdump tutorial.joedb >logdump.out
$dir/joedb_logdump --sql tutorial.joedb >logdump.sql
$dir/joedb_to_json tutorial.joedb >tutorial.json
echo help | $dir/joedbi >joedbi_help.out

rm -f merge_1.joedb merge_2.joedb merged.joedb
$dir/joedbi merge_1.joedb <merge_1.joedbi >/dev/null
$dir/joedbi merge_2.joedb <merge_2.joedbi >/dev/null
$dir/joedb_merge merge_1.joedb merge_2.joedb merged.joedb
$dir/joedb_to_json merge_1.joedb >merge_1.json
$dir/joedb_to_json merge_2.joedb >merge_2.json
$dir/joedb_to_json merged.joedb >merged.json

set +e
( cd $dir && PATH="." && joedb_merge ) 2>joedb_merge.out
( cd $dir && PATH="." && joedb_embed ) 2>joedb_embed.out
( cd $dir && PATH="." && joedb_ssh_connect ) 2>joedb_ssh_connect.out
( cd $dir && PATH="." && joedb_logdump ) 2>joedb_logdump.out
