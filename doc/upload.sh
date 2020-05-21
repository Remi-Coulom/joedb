#!/bin/bash
make clean
make html

cd build/html
rsync -v -r --delete . rcoulom@www.remi-coulom.fr:web/www/joedb
