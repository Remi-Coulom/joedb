#!/bin/bash

for path in `update-alternatives --list gcc`; do
 echo
 echo "=================================="
 echo $path
 echo "=================================="
 sudo update-alternatives --set gcc $path
 ./bug.sh
done
