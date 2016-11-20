wget https://github.com/Remi-Coulom/joedb/archive/vVERSION.tar.gz
tar zxvf vVERSION.tar.gz
cd joedb-VERSION/compcmake/
./get_gtest.sh
./generate.sh
cd ninja_release/
ninja
