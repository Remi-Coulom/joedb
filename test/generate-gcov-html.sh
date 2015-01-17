#!/bin/bash
lcov --directory ../compcmake/ninja_debug --capture --output-file all.info || exit 1
lcov  --remove all.info '/usr/*' --output app.info || exit 1
c++filt -n <app.info >demangled.info
mkdir -p html
genhtml --output-directory html demangled.info || exit 1
echo xdg-open html/index.html
