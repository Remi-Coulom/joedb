#!/bin/bash
TZ='Europe/Paris'
cd ../compcmake/ninja_coverage
ninja clean
rm -vf `find . -name \*.gcda`
rm -vf `find . -name \*.gcno`
ninja joedb_test || exit 1
cd -

../compcmake/ninja_coverage/joedb_test || exit 1

read -p "Enter to continue..."
./generate-gcov-html.sh
