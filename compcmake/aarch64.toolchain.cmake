# sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu qemu-system-arm qemu-user gdb-multiarch

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR
 "/usr/bin/qemu-aarch64;-L;/usr/aarch64-linux-gnu"
)
