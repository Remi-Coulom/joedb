#!/bin/bash
set -e

mkdir -p build
cd build
cmake ..
make -j 12
cd -

configuration=gcc_debug
dir=../../../compcmake/$configuration
cd ../../../compcmake
./generate.sh $configuration
cd -
cmake --build $dir

rm -f tutorial.joedb index_tutorial.joedb local_concurrency.joedb local_concurrency_?.txt

$dir/index_tutorial >index_tutorial.out
$dir/tutorial >tutorial.out
$dir/concurrency_tutorial >concurrency_tutorial.out

$dir/joedb_logdump tutorial.joedb >logdump.joedbi
$dir/joedb_logdump --sql tutorial.joedb >logdump.sql
$dir/joedb_to_json tutorial.joedb >tutorial.json
echo "echo off
help" | $dir/joedbi memory>joedbi_help.out

rm -f merge_1.joedb merge_2.joedb merged.joedb
$dir/joedbi --new merge_1.joedb <merge_1.joedbi >/dev/null
$dir/joedbi --new merge_2.joedb <merge_2.joedbi >/dev/null
$dir/joedb_merge merge_1.joedb merge_2.joedb merged.joedb >/dev/null 2>/dev/null
$dir/joedb_to_json merge_1.joedb >merge_1.json
$dir/joedb_to_json merge_2.joedb >merge_2.json
$dir/joedb_to_json merged.joedb >merged.json

set +e
( cd $dir && PATH="." && joedbi ) 2>joedbi.out
( cd $dir && PATH="." && joedb_merge ) 2>joedb_merge.out
( cd $dir && PATH="." && joedb_embed ) 2>joedb_embed.out
( cd $dir && PATH="." && joedb_logdump ) 2>joedb_logdump.out
( cd $dir && PATH="." && joedbc ) 2>joedbc.out
( cd $dir && PATH="." && joedb_convert ) 2>joedb_convert.out
( cd $dir && PATH="." && joedb_pack ) 2>joedb_pack.out
( cd $dir && PATH="." && joedb_server ) 2>joedb_server.out
( cd $dir && PATH="." && joedb_multi_server ) 2>joedb_multi_server.out
( cd $dir && PATH="." && joedb_client ) 2>joedb_client.out
( cd $dir && PATH="." && joedb_push ) 2>joedb_push.out

echo "done"
