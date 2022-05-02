#!/bin/bash
set -e

mkdir -p system-headers
mkdir -p system-headers/sys

for include in algorithm array cstdint cstring errno.h exception fcntl.h limits sstream stdint.h string cmath deque map stddef.h stdio.h string.h vector cstddef iostream memory stdexcept stdlib.h x86intrin.h iosfwd cassert utility random assert.h cctype iomanip ostream thread mutex condition_variable ctime sys/file.h sys/stat.h sys/types.h unistd.h fstream signal.h; do
# echo "include <$include>" >system-headers/$include
 touch system-headers/$include
done

g++ -I../../../../src -E -nostdinc -Dinclude=#include -I. -Isystem-headers bug.cpp | grep --binary-files=text -v '^# [0-9]' | sed -e '1 i #include"all_includes.h"'>repro.cpp
