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
