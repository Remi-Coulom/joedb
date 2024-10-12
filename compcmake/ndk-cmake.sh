#!/bin/bash
NDK=~/android-ndk-r27b
ABI=arm64-v8a
MINSDKVERSION=29
cmake \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$ABI \
    -DANDROID_PLATFORM=android-$MINSDKVERSION \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja $*

# Download of Linux ndk
# https://developer.android.com/ndk/downloads
# wget https://dl.google.com/android/repository/android-ndk-r26b-linux.zip

# problems: custom commands that require running a target do not run
# will run if installed system-wide in /usr/local/bin
# solution: run ../install_bin.sh before in gcc_release

# problem: for some reason, precompiled headers seem to not work in NDK
# solution: they are disabled when compiling for Android

# Building with ndk, in compcmake
# mkdir ndk
# cd ndk
# ../ndk-cmake.sh ..

# path to adb:
# Windows: C:\Users\rcoulom\AppData\Local\Android\Sdk\platform-tools\adb.exe
# wsl: /mnt/c/Users/rcoulom/AppData/Local/Android/Sdk/platform-tools/adb.exe

# running a binary on the device:

# first make a dir for joedb binaries:
# adb shell
# redfin:/ $ cd /data/local/tmp
# redfin:/data/local/tmp $ mkdir joedb
# redfin:/data/local/tmp $ cd joedb/
# redfin:/data/local/tmp/joedb $

# upload binaries with adb:
# adb push libjoedb_for_joedbc.so /data/local/tmp/joedb
# adb push libjoedb.so /data/local/tmp/joedb
# adb push joedbi /data/local/tmp/joedb

# configure and run in adb shell:
# redfin:/data/local/tmp/joedb $ chmod a+x ./joedbi ./*.so
# redfin:/data/local/tmp/joedb $ export LD_LIBRARY_PATH=.
# redfin:/data/local/tmp/joedb $ echo "about" | ./joedbi
