#!/bin/bash
cd ../compcmake/ninja_debug
ninja clean
rm -vf `find -name *.gcda`
rm -vf `find -name *.gcno`
ninja joedb_test
cd -
valgrind ../compcmake/ninja_debug/joedb_test || exit 1
read -p "Enter to continue..."
./generate-gcov-html.sh
