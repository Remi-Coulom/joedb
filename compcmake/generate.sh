#!/bin/bash

favorite_compiler=g++
compiler_option=""
compiler_path=`which $favorite_compiler`
if [ "$compiler_path" != "" ]; then
 echo Found $favorite_compiler: $compiler_path
 compiler_option="-DCMAKE_CXX_COMPILER=$compiler_path"
else
 echo $favorite_compiler not found
fi
echo compiler_option:$compiler_option

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

generate "$build_system_prefix"release cmake $build_system -DCMAKE_BUILD_TYPE=Release $compiler_option
generate "$build_system_prefix"debug cmake $build_system -DCMAKE_BUILD_TYPE=Debug $compiler_option
generate "$build_system_prefix"coverage cmake $build_system -DCMAKE_BUILD_TYPE=Coverage $compiler_option

if [ $OSTYPE == "cygwin" ]; then
# generate mingw32 cmake -G Ninja -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++
# generate mingw64 cmake -G Ninja -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
# generate vs2013 cmake_win -G "Visual Studio 12 2013"
 generate vs2013-64 cmake_win -G "Visual Studio 12 2013 Win64"
 generate vs2015-64 cmake_win -G "Visual Studio 14 2015 Win64"
# generate vs2013-WindowsPhone cmake_win -G "Visual Studio 12 2013" -DCMAKE_SYSTEM_NAME=WindowsPhone -DCMAKE_SYSTEM_VERSION=8.1
# generate vs2013-WindowsStore cmake_win -G "Visual Studio 12 2013" -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=8.1
fi
