#!/bin/bash
set -e

cd ../compcmake
./generate.sh
configurations=`find . -maxdepth 1 -mindepth 1 -type d -printf ' %f'`
cd -

echo configurations:${configurations}

export WINEPATH="/usr/x86_64-w64-mingw32/lib\;/usr/lib/gcc/x86_64-w64-mingw32/9.3-posix"

for configuration in ${configurations}; do
 echo =======================================================================
 echo ${configuration}
 echo =======================================================================
 build_dir=../compcmake/${configuration}
 cd $build_dir
 ctest -V
 cd -
done
