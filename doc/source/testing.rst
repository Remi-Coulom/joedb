Testing
=======

Tests are located in the ``test`` directory. There you can run:

  - ``./test.sh`` run the tests
  - ``./coverage.sh`` run tests, and produce coverage data with gcov.
  - The ``fuzz`` subdirectory contains fuzzers, and their corpora. The fuzzers
    use llvm libFuzzer, and are compiled in the clang_release cmake preset.

Compiling libc++ with memory sanitizer in Ubuntu 24.04 (https://github.com/google/sanitizers/issues/1815):

.. code-block:: bash

   sudo apt install clang-19 libllvmlibc-19-dev libclang-19-dev clang-tidy-19
   sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100
   sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100
   sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-19 100
   git clone --depth=1 https://github.com/llvm/llvm-project
   cd llvm-project
   mkdir build
   cmake -GNinja -S ../runtimes \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
    -DLIBCXXABI_USE_LLVM_UNWINDER=off \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DLLVM_USE_SANITIZER=MemoryWithOrigins
