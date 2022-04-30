#!/bin/bash
set -e
set -o xtrace

# Generate .ii file for bug report
g++ -save-temps -DNDEBUG -I ../../../../src -o bug bug.cpp

# Compiling without LTO works OK
g++ -O3 -o bug bug.ii
./bug

# LTO produces an infinite loop
g++ -O3 -flto -o bug bug.ii
echo This is likely to hang in an infinite loop...
./bug
echo Yeah, no bug!
