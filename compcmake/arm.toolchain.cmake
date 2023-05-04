# https://github.com/WojciechMigda/how-to-qemu-arm-gdb-gtest
# sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf qemu-system-arm qemu-user gdb-multiarch

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_C_FLAGS "-march=armv7-a -mfloat-abi=hard -mfpu=neon")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR
 "/usr/bin/qemu-arm;-L;/usr/arm-linux-gnueabihf"
)