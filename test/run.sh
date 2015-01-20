#!/bin/bash
interpreter=../compcmake/ninja_debug/interpreter

cd ..
rm -vf `find -name *.gcda`
cd -

valgrind "$interpreter" <test1.joedbi >test1.tmp

if diff -w test1.out test1.tmp; then
 echo result is OK
else
 echo Error!
fi
