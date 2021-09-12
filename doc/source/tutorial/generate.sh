#!/bin/bash
set -e

dir=../../../compcmake/gcc_debug
cmake --build $dir

rm -f tutorial.joedb index_tutorial.joedb

$dir/index_tutorial >index_tutorial.out
$dir/tutorial >tutorial.out
$dir/concurrency_tutorial >concurrency_tutorial.out

$dir/joedb_logdump tutorial.joedb >logdump.joedbi
$dir/joedb_logdump --sql tutorial.joedb >logdump.sql
$dir/joedb_to_json tutorial.joedb >tutorial.json
echo "echo off
help" | $dir/joedbi >joedbi_help.out

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
( cd $dir && PATH="." && joedb_logdump ) 2>joedb_logdump.out
( cd $dir && PATH="." && joedbc ) 2>joedbc.out
( cd $dir && PATH="." && joedb_pack ) 2>joedb_pack.out
( cd $dir && PATH="." && joedb_server ) 2>joedb_server.out
( cd $dir && PATH="." && joedb_multi_server ) 2>joedb_multi_server.out
( cd $dir && PATH="." && joedb_network_client ) 2>joedb_network_client.out
( cd $dir && PATH="." && joedb_ssh_client ) 2>joedb_ssh_client.out
( cd $dir && PATH="." && joedb_local_client ) 2>joedb_local_client.out
