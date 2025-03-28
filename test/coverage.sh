#!/bin/bash
set -e

if [ ! -d "../compcmake/gcc_coverage" ]; then
 cd ../compcmake
 ./generate.sh gcc_coverage
 cd -
fi

TZ='Europe/Paris'
cd ../compcmake/gcc_coverage
cmake --build . --target clean
rm -vf `find . -name \*.gcda`
rm -vf `find . -name \*.gcno`
cmake --build . --target joedb_test || exit 1
cd -

rm -vf *.tmp
../compcmake/gcc_coverage/joedb_test || exit 1

./generate-gcov-html.sh
