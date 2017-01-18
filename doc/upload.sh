#!/bin/bash
make clean
cd ./source/tutorial
./generate.sh
cd -
make html

cd build/html
rsync -v -r --delete . rcoulom@www.remi-coulom.fr:web/www/joedb
