#!/bin/bash
echo Generating cmake build directories...

git submodule update --init --recursive

build_system=""
ninja_path=`which ninja`
if [ "$ninja_path" != "" ]; then
 build_system="-G Ninja"
fi

echo
echo =======================================================================
echo ninja_path=$ninja_path
echo build_system_prefix=$build_system_prefix
echo build_system=$build_system

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

echo
echo =======================================================================
echo gpp_path=$gpp_path
echo gcc_path=$gcc_path

if [ "$gcc_path" != "" ]; then
 compiler="-DCMAKE_CXX_COMPILER=$gpp_path -DCMAKE_C_COMPILER=$gcc_path"
 generate gcc_release cmake $build_system -DCMAKE_BUILD_TYPE=Release $compiler
 generate gcc_debug cmake $build_system -DCMAKE_BUILD_TYPE=Debug $compiler
 generate gcc_portable cmake $build_system -DCMAKE_BUILD_TYPE=Debug -DJOEDB_PORTABLE=TRUE $compiler
 generate gcc_coverage cmake $build_system -DCMAKE_BUILD_TYPE=Coverage $compiler
fi

clangpp_path=`which clang++`
clang_path=`which clang`

echo
echo =======================================================================
echo clangpp_path=$clangpp_path
echo clang_path=$clang_path

if [ "$clang_path" != "" ]; then
 generate clang_release cmake $build_system -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER="$clangpp_path" -DCMAKE_C_COMPILER="$clang_path"
fi
