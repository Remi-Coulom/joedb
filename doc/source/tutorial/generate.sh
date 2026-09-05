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

set +e
( cd $dir && PATH="." && joedbi ) 2>joedbi.out
( cd $dir && PATH="." && joedb_logdump ) 2>joedb_logdump.out
( cd $dir && PATH="." && joedbc ) 2>joedbc.out
( cd $dir && PATH="." && joedb_convert ) 2>joedb_convert.out
( cd $dir && PATH="." && joedb_pack ) 2>joedb_pack.out
( cd $dir && PATH="." && joedb_server ) 2>joedb_server.out
( cd $dir && PATH="." && joedb_multi_server ) 2>joedb_multi_server.out
( cd $dir && PATH="." && joedb_client ) 2>joedb_client.out
( cd $dir && PATH="." && joedb_push ) 2>joedb_push.out

echo "done"
