#!/bin/bash
#~/emsdk_portable/emsdk activate latest
source ~/emsdk_portable/emsdk_env.sh
emcc -s ASYNCIFY=1 -Oz --bind -o embind_tutorial.js embind_tutorial.cpp
