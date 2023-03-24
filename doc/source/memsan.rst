Reference: https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
But it does not work. cmake fails.
Better information available there: https://libcxx.llvm.org/BuildingLibcxx.html

Steps that work on Ubuntu 20.04.5 LTS:
cd ~/repos
git clone --depth=1 https://github.com/llvm/llvm-project
cd llvm-project
mkdir build
cmake -GNinja -S runtimes -B build -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLLVM_USE_SANITIZER=MemoryWithOrigins
ninja -C build cxx cxxabi unwind
ninja -C build check-cxx check-cxxabi check-unwind

