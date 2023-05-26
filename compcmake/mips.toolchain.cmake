# https://github.com/WojciechMigda/how-to-qemu-arm-gdb-gtest
# sudo apt install gcc-multilib-mips-linux-gnu g++-multilib-mips-linux-gnu gcc-mips-linux-gnu qemu-user

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)

set(CMAKE_C_COMPILER mips-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER mips-linux-gnu-g++)

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR "/usr/bin/qemu-mips;-L;/usr/mips-linux-gnu")
