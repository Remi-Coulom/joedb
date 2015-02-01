#!/bin/bash
make clean
make html
cd build/html
lftp -e "cd joedb; mirror -e -R;quit" remi.coulom@ftpperso.free.fr
