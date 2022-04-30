#!/bin/bash
set -e
set -o xtrace

# Generate .ii file for bug report
g++ -save-temps -I ../../../../src -o local_concurrency local_concurrency_unity_build.cpp
mv local_concurrency-local_concurrency_unity_build.ii bug.ii

# Compiling without LTO works OK
g++ -O3 -o local_concurrency bug.ii
rm -f local_concurrency.joedb
./local_concurrency

# LTO produces an infinite loop
g++ -O3 -flto -o local_concurrency bug.ii
rm -f local_concurrency.joedb
echo This is likely to hang in an infinite loop...
./local_concurrency
echo Yeah, no bug!
