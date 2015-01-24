#!/bin/bash
cd ../compcmake/ninja_debug
#ninja clean
#rm -vf `find -name *.gcda`
#rm -vf `find -name *.gcno`
#ninja joedb_test
cd -

if [ $OSTYPE == Linux ]; then
 echo Linux
 valgrind ../compcmake/ninja_debug/joedb_test || exit 1
else
 echo Not Linux
 ../compcmake/ninja_debug/joedb_test || exit 1
fi

read -p "Enter to continue..."
./generate-gcov-html.sh
