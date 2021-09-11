#!/bin/bash
set -e

ninja_path=`which ninja`
if [ "$ninja_path" != "" ]; then
 build_system_prefix="ninja_"
 build_system="-G Ninja"
else
 build_system=""
 build_system_prefix=""
 echo Warning: ninja not available!
fi

function generate {
 echo
 echo "====> Generating $1 ..."
 mkdir -p $1
 cd $1
 shift
 "$@" ..
 cd ..
}

gpp_path=`which g++`
gcc_path=`which gcc`
if [ "$gcc_path" != "" ]; then
 compiler="-DCMAKE_CXX_COMPILER=$gpp_path -DCMAKE_C_COMPILER=$gcc_path"
 generate "$build_system_prefix"release cmake $build_system -DCMAKE_BUILD_TYPE=Release $compiler
 generate "$build_system_prefix"debug cmake $build_system -DCMAKE_BUILD_TYPE=Debug $compiler
 generate "$build_system_prefix"coverage cmake $build_system -DCMAKE_BUILD_TYPE=Coverage $compiler
fi

clangpp_path=`which clang++`
clang_path=`which clang`
if [ "$clang_path" != "" ]; then
 generate "$build_system_prefix"clang cmake $build_system -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="$clangpp_path" -DCMAKE_C_COMPILER="$clang_path"
fi
