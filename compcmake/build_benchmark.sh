#!/bin/bash
# Compilation-time tracing: https://github.com/nico/ninjatracing
# ~/repos/ninjatracing/ninjatracing.py .ninja_log >trace.json
# Open json trace file in https://ui.perfetto.dev/
set -e
dir=$1

if [ "$dir" == "" ]; then
 exit 1
fi

rm -rvf $dir
./generate.sh $dir
cd $dir
time ninja
cd -
