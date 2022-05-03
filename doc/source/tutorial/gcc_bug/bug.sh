#!/bin/bash

#
#  - Ubuntu 20.04:
#    * 7.5.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached a __builtin_unreachable() call
#    * 8.4.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached an unreachable program point
#    * 9.4.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached an unreachable program point
#    * 10.3.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached an unreachable program point
#    * 11.1.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached an unreachable program point
#  - Cgywin (32 bits)
#    * 11.2.0 -> infinite loop (but no ubsan)
#
# -O2 instead of -O3 works well
#
# In repro.cpp, not catching exceptions in main makes the bug disappear.
#
# Without -fsanitize=undefined, the program either runs an infinite loop, or crashes.
#
# Compiling with -flto produces one strange warning with g++ version < 11:
# ../../../../src/joedb/journal/Readonly_Journal.cpp:396:22: warning:  may be used uninitialized in this function [-Wmaybe-uninitialized]
#  396 |   return Type(type_id);
# It is strange, because it seems that the unused variable name is empty.
# This strange warning disappeared with 11.1.0, so it may be an unrelated bug that has been fixed already.
#
# This bug may be unrelated to -flto: in some other (confidential) code I wrote, an infinite loop occurred without -flto.
#
# The crash or infinite loop occurs in the constructor of joedb::Client.
#
# Bug with vaguely similar symptoms, but may be unrelated:
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87525
#

set -e
g++ --version
rm -f local_concurrency.joedb

OPTIONS="-Wall -Wextra -Wno-unused-parameter -fno-strict-aliasing -fwrapv -fno-aggressive-loop-optimizations -g"
if [ "$OSTYPE" != "cygwin" ]; then
 OPTIONS="${OPTIONS} -fsanitize=unreachable"
fi

set -o xtrace

# Generate .ii file for bug report
g++ -save-temps -DNDEBUG -I ../../../../src ${OPTIONS} -O0 -o repro repro.cpp
./repro

# Compiling without LTO works OK
g++ ${OPTIONS} -O3 -o repro repro.ii
./repro

# LTO usually does not work well
g++ ${OPTIONS} -O3 -flto=auto -o repro repro.ii
./repro
