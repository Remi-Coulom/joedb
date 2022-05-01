#!/bin/bash

# to install version XX:
# sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-XX 100 --slave /usr/bin/g++ g++ /usr/bin/g++-XX --slave /usr/bin/gcov gcov /usr/bin/gcov-XX

for path in `update-alternatives --list gcc`; do
 echo
 echo "=================================="
 echo $path
 echo "=================================="
 sudo update-alternatives --set gcc $path
 ./bug.sh
done
