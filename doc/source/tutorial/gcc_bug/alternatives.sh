#!/bin/bash

# to install version XX:
# sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-XX 100 --slave /usr/bin/g++ g++ /usr/bin/g++-XX --slave /usr/bin/gcov gcov /usr/bin/gcov-XX
# for gcc experimental version:
# sudo update-alternatives --install /usr/bin/gcc gcc /home/rcoulom/repos/gcc_install/usr/local/bin/gcc 100 --slave /usr/bin/g++ g++ /home/rcoulom/repos/gcc_install/usr/local/bin/g++ --slave /usr/bin/gcov gcov /home/rcoulom/repos/gcc_install/usr/local/bin/gcov

for path in `update-alternatives --list gcc`; do
 echo
 echo "==============================================================================="
 echo $path
 echo "==============================================================================="
 sudo update-alternatives --set gcc $path
 ./bug.sh
done
