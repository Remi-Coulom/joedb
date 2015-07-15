#!/bin/bash
make clean
make html

#cd build/html
#lftp -e "cd joedb; mirror -e -R;quit" remi.coulom@ftpperso.free.fr

cd build/html
rsync -v -r --delete . rcoulom@www.remi-coulom.fr:web/www/joedb
