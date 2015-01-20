#!/bin/bash
cd ../compcmake/ninja_debug
ninja clean
rm -vf `find -name *.gcda`
rm -vf `find -name *.gcno`
ninja gtest
cd -
valgrind ../compcmake/ninja_debug/gtest || exit 1
read -p "Enter to continue..."
./generate-gcov-html.sh
