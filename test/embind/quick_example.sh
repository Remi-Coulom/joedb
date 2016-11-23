#!/bin/bash
#~/emsdk_portable/emsdk activate latest
source ~/emsdk_portable/emsdk_env.sh
emcc -Oz --bind -o quick_example.js quick_example.cpp
