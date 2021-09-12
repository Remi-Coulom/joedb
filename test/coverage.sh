#!/bin/bash
TZ='Europe/Paris'
cd ../compcmake/gcc_coverage
cmake --build . --target clean
rm -vf `find . -name \*.gcda`
rm -vf `find . -name \*.gcno`
cmake --build . --target joedb_test || exit 1
cd -

../compcmake/gcc_coverage/joedb_test || exit 1

read -p "Enter to continue..."
./generate-gcov-html.sh
