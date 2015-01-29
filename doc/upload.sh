#!/bin/bash
make html
cd build/html
lftp -e "cd joedb; mirror -R;quit" remi.coulom@ftpperso.free.fr
