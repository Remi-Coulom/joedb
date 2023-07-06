#!/bin/bash

apt install g++ clang

dpkg --add-architecture i386
apt install libc6-dev:i386

apt install g++-arm-linux-gnueabihf
# apt install g++-mips-linux-gnu
apt install g++-mingw-w64-x86-64
