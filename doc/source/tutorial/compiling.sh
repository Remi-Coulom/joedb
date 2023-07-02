git submodule update --init --recursive
cd joedb/compcmake/
./generate.sh
cd gcc_release/
cmake --build .
