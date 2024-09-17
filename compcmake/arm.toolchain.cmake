# https://github.com/WojciechMigda/how-to-qemu-arm-gdb-gtest
# sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf qemu-system-arm qemu-user gdb-multiarch

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi") #https://stackoverflow.com/questions/48149323/what-does-the-gcc-warning-project-parameter-passing-for-x-changed-in-gcc-7-1-m

set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_CROSSCOMPILING_EMULATOR
 "/usr/bin/qemu-arm;-L;/usr/arm-linux-gnueabihf"
)
