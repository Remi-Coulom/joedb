#!/bin/bash
set -e
cd ../..
version=$(<joedb/VERSION)
distro_dir=joedb-${version//\"}
archive="$distro_dir".tar.bz2
git clone joedb "$distro_dir"
tar --exclude=.git -j -c -v -f "$archive" "$distro_dir"
scp $archive rcoulom@www.remi-coulom.fr:web/www/joedb
