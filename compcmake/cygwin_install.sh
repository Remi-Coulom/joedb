#!/bin/bash
#This script installs binary compiled by Visual Studio into the cygwin path
source=out/install/x64-Release
target=/usr/local
rm -vf $target/bin/joedb*.exe
cp -v $source/bin/joedb*.exe $target/bin/
rm -rvf $target/include/joedb
cp -vR $source/include/joedb $target/include
rm -vf $target/lib/joedb*.lib
cp -v $source/lib/joedb*.lib $target/lib
