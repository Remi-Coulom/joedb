#!/bin/bash
#~/emsdk_portable/emsdk activate latest
source ~/emsdk_portable/emsdk_env.sh
emcc -I/usr/local/include -s ASYNCIFY=1 -Oz --bind -o embind_tutorial.js embind_tutorial.cpp ../../doc/source/tutorial/tutorial.cpp ../../../joedb/src/compiler/joedbc_lib.cpp
