#!/bin/bash

#
#  - Ubuntu 20.04:
#    * 7.5.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached a __builtin_unreachable() call
#    * 8.4.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached an unreachable program point
#    * 9.4.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached an unreachable program point
#    * 10.3.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached an unreachable program point
#    * 11.1.0 -> ../../../../src/joedb/journal/Readonly_Journal.h:87:24: runtime error: execution reached an unreachable program point
#  - Cgywin (32 bits)
#    * 11.2.0 ->
#
# It works fine with clang or msvc.
#
# Compiling without -flto does not produce any warning, but -flto has one:
# ../../../../src/joedb/journal/Readonly_Journal.cpp:396:22: warning:  may be used uninitialized in this function [-Wmaybe-uninitialized]
#  396 |   return Type(type_id);
# It is strange, because it seems that the unused variable name is empty.
#
# In bug.cpp, not catching exceptions in main makes the bug disappear (but not the strange warning).
#
# With OPTIONS="", the program either runs an infinite loop, or crashes.
#
# Symptoms are a bit similar to:
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87525
# infinite loop generated for fread() if enabling -flto and -D_FORTIFY_SOURCE=2
# But I am not sure it is the same problem
#

set -e
g++ --version
set -o xtrace

OPTIONS="-Wall -Wextra -Wno-unused-parameter -fno-strict-aliasing -fwrapv -fno-aggressive-loop-optimizations -fsanitize=undefined"
#OPTIONS=""

# Generate .ii file for bug report
g++ -save-temps -DNDEBUG -I ../../../../src ${OPTIONS} -o bug bug.cpp

# Compiling without LTO works OK
g++ -O3 ${OPTIONS} -o bug bug.ii
./bug

# LTO usually does not work well
g++ -O3 -flto ${OPTIONS} -o bug bug.ii
echo This is likely to crash or hang in an infinite loop...
./bug
echo Yeah, no bug!
