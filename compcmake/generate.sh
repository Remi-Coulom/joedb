#!/bin/bash
echo Generating cmake build directories...
config="$1"

# For some old versions of git, this must be executed at the root
cd ..
git submodule update --init --recursive
cd -

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
 if [[ "$config" == "" ]] || [[ "$config" == "$1" ]]; then
  echo
  echo "====> Generating $1 ..."
  mkdir -p $1
  cd $1
  shift
  echo "$@"
  "$@" ..
  cd ..
 fi
}

echo
echo =======================================================================

gpp_path=`which g++`
gcc_path=`which gcc`

echo gpp_path=$gpp_path
echo gcc_path=$gcc_path

if [ "$gcc_path" != "" ]; then
 compiler="-DCMAKE_CXX_COMPILER=$gpp_path -DCMAKE_C_COMPILER=$gcc_path"
 generate gcc_release cmake $build_system -DCMAKE_BUILD_TYPE=Release $compiler
 generate gcc_debug cmake $build_system -DCMAKE_BUILD_TYPE=Debug $compiler
 generate gcc_portable cmake $build_system -DCMAKE_BUILD_TYPE=Debug -DJOEDB_PORTABLE=TRUE $compiler
 generate gcc_coverage cmake $build_system -DCMAKE_BUILD_TYPE=Coverage $compiler
 generate gcc_asan cmake $build_system -DCMAKE_BUILD_TYPE=ASAN $compiler
 generate gcc_tsan cmake $build_system -DCMAKE_BUILD_TYPE=TSAN $compiler
 #note: fore 32-compilation, apt install gcc-multilib
 generate gcc_debug32 cmake $build_system -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 $compiler
fi

echo
echo =======================================================================

clangpp_path=`which clang++`
clang_path=`which clang`

echo clangpp_path=$clangpp_path
echo clang_path=$clang_path

if [ "$clang_path" != "" ]; then
 compiler="-DCMAKE_CXX_COMPILER=$clangpp_path -DCMAKE_C_COMPILER=$clang_path"
 generate clang_release cmake $build_system -DCMAKE_BUILD_TYPE=Release $compiler
 generate clang_msan cmake $build_system -DCMAKE_BUILD_TYPE=MSAN $compiler
fi

echo
echo =======================================================================
arm_gcc_path=`which arm-linux-gnueabihf-gcc`
echo arm_gcc_path=$arm_gcc_path

if [ "$arm_gcc_path" != "" ]; then
 generate arm_release cmake $build_system -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=`dirname "$0"`/arm.toolchain.cmake
fi
