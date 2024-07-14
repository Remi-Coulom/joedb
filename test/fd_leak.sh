#!/bin/bash
set -e
config=gcc_debug
joedb_test=../compcmake/$config/joedb_test
valgrind --track-fds=yes $joedb_test --gtest_filter=File_Test.open_lock
