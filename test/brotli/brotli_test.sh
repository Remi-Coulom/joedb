#!/bin/bash
set -e

#
# Start by compiling joedb_push
#
this_dir=$(pwd)
cd ../../compcmake
if [ ! -d gcc_dev ]; then
 ./generate.sh gcc_dev
fi
cd gcc_dev
ninja joedb_push
cd $this_dir

#
# Compress a large file
#
rm -vf compressed.joedb decompressed.joedb
echo
echo Compression...
time ../../compcmake/gcc_dev/joedb_push original.joedb file brotli compressed.joedb
echo
echo Decompression...
time ../../compcmake/gcc_dev/joedb_push brotli compressed.joedb file decompressed.joedb
echo
ls -lL *.joedb
