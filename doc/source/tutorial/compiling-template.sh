wget https://www.remi-coulom.fr/joedb/joedb-VERSION.tar.bz2
tar jxvf joedb-VERSION.tar.bz2
cd joedb-VERSION/compcmake/
./get_gtest.sh
./generate.sh
cd ninja_release/
ninja
