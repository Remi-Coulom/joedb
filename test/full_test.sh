#!/bin/bash
set -e

if [ "$1" == "" ]; then
 cd ../compcmake
 ./generate.sh
 configurations=`find . -maxdepth 1 -mindepth 1 -type d -printf ' %f'`
 cd -
else
 if [ "$1" == "usual" ]; then
  configurations="gcc_asan gcc_debug gcc_debug32 gcc_portable clang_release arm_release"
 else
  configurations=$*
 fi

 echo configurations: ${configurations}
 sleep 2
fi

for configuration in ${configurations}; do
 echo =======================================================================
 echo ${configuration}
 echo =======================================================================
 cd ../compcmake
 ./generate.sh $configuration
 cd -
 build_dir=../compcmake/${configuration}
 cd $build_dir
 ctest -V
 cd -
done
