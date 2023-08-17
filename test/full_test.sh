#!/bin/bash
set -e

if [ "$1" == "" ]; then
 cd ../compcmake
 ./generate.sh
 configurations=`find . -maxdepth 1 -mindepth 1 -type d -printf ' %f'`
 cd -
else
 if [ "$1" == "usual" ]; then
  configurations="gcc_asan gcc_debug gcc_portable clang_release arm_release clang_release"
 else
  configurations=$*
 fi

 echo configurations: ${configurations}
 sleep 2
fi

export WINEPATH="/usr/x86_64-w64-mingw32/lib\;/usr/lib/gcc/x86_64-w64-mingw32/9.3-posix"

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
