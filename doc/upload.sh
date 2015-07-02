#!/bin/bash
make clean
make html

#cd build/html
#lftp -e "cd joedb; mirror -e -R;quit" remi.coulom@ftpperso.free.fr

cd build
mv -v html joedb
rsync -r --delete joedb rcoulom@www.remi-coulom.fr:web/www
mv -v joedb html
