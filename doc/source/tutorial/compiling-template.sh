wget https://github.com/Remi-Coulom/joedb/archive/VERSION.tar.gz
tar zxvf VERSION.tar.gz
cd joedb-VERSION/compcmake/
./get_gtest.sh
./generate.sh
cd ninja_release/
ninja
