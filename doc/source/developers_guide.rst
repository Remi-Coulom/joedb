Developer's Guide
=================

Running Tests
-------------

Tests are located in the ``test`` directory. There you can run:

  - ``./test.sh`` run the tests
  - ``./coverage.sh`` run tests, and produce coverage data with gcov.
  - The ``fuzz`` subdirectory contains 3 fuzzers, and their corpus. The fuzzers
    use llvm libFuzzer, and are compiled in the clang_release cmake preset.

Understanding Source Code
-------------------------

For the moment, there is no reference documentation, except for the source code itself.

Recommended path to understanding the source:

  - ``src/joedb``:

    - ``Readable.h`` and ``Writable.h`` define the most important base classes.
    - In addition, a few utilities, mainly for error management.

  - ``src/joedb/interpreter``: classes to store a database in memory, an
    implementation of the abstract Readable and Writable.
  - ``src/joedb/journal``: for reading and writing journals to files

    - First ``Generic_File`` is a bit like joedb's std::iostream.
    - A few specializations, for Windows, Posix, in-memory, ...
    - Then ``Readonly_Journal`` and ``Writable_Journal`` are the main classes.

  - Previous elements are enough to understand ``joedbi``, located in
    ``src/joedb/io/joedbi.cpp``, like other command-line tools.
  - ``src/compiler``: all code related to ``joedbc``
  - ``src/concurrency``: concurrency
  - ``src/ssh``: wrappers around the libssh API to provide the ssh
    port-forwarding channel.
