#!/bin/bash
set -e

for i in db/*.joedbi; do
 b=${i%.joedbi}
 echo "will remove $b"
 read
 rm -rvf $b
done

sources="$(find -name "*.h") $(find -name "*.c") $(find -name "*.hpp") $(find -name "*.cpp")"
echo $sources
echo "Number of lines:"
cat $sources | wc -l
echo "Size of gzipped source without comments:"
cat $sources | gcc -fpreprocessed -w -dD -E - | gzip | wc -c
