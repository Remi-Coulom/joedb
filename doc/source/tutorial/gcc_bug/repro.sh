#!/bin/bash
set -e
set -o xtrace
g++ -O3 -Wall -flto=auto -fsanitize=unreachable -o repro repro.cpp
./repro
