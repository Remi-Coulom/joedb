Testing
=======

Tests are located in the ``test`` directory. There you can run:

  - ``./test.sh`` run the tests
  - ``./coverage.sh`` run tests, and produce coverage data with gcov.
  - The ``fuzz`` subdirectory contains fuzzers, and their corpora. The fuzzers
    use llvm libFuzzer, and are compiled in the clang_release cmake preset.
