#!/bin/bash
set -e

cd ../compcmake
./generate.sh
configurations=`find . -maxdepth 1 -mindepth 1 -type d -printf ' %f'`
cd -

echo configurations:${configurations}

for configuration in ${configurations}; do
 echo =======================================================================
 echo ${configuration}
 echo =======================================================================
 build_dir=../compcmake/${configuration}
 cd $build_dir
 ctest -V
 cd -
done
