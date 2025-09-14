#!/bin/bash
set -e

if [ -d db ]; then
 cd db
 for i in *.joedbi; do
  b=$(basename $i .joedbi)
  rm -vf $b
 done
 cd -
fi

sources="$(find -name "*.h") $(find -name "*.c") $(find -name "*.hpp") $(find -name "*.cpp")"
echo $sources
echo "Number of lines:"
cat $sources | wc -l
echo "Size of gzipped source without comments:"
cat $sources | gcc -fpreprocessed -w -dD -E - | gzip | wc -c
