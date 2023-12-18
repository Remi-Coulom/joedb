#!/bin/bash
NDK=~/android-ndk-r26b
ABI=arm64-v8a
MINSDKVERSION=29
cmake \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$ABI \
    -DANDROID_PLATFORM=android-$MINSDKVERSION \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja $*
