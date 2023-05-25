#!/bin/bash
set -e

cd ../compcmake
configurations=`find . -maxdepth 1 -mindepth 1 -type d -printf ' %f'`
cd -

echo configurations:${configurations}

for configuration in ${configurations}; do
 echo =======================================================================
 echo ${configuration}
 echo =======================================================================
 build_dir=../compcmake/${configuration}
 cmake --build $build_dir
 rm -vf *.tmp
 if [ -x $build_dir/joedb_test ]; then
  $build_dir/joedb_test --gtest_shuffle
 fi
done
