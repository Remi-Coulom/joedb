History
=======

- 2021-08-??: 6.0

  - Re-organize file classes:

    - ``joedb::File_Slice`` is removed. All file classes can be sliced now.
    - ``joedb::Stream_File`` uses a ``std::streambuf`` instead of ``std::iostream``.

  - :ref:`joedb_multi_server`.
  - Minor fixes and improvements.

- 2021-05-08: 5.0

  - Big improvements to :doc:`concurrency <concurrency>`: joedb now has a
    :doc:`network protocol <network_protocol>`, and a :ref:`server
    <joedb_server>` for efficient and reliable communication.
  - The old serverless ``SSH_Connection`` was removed completely. It was
    inefficient and unreliable. Using the server is much better.
  - Performance improvements of operations on large :doc:`vectors <vectors>`.
  - vim syntax highlighting for ``joedbi`` and ``joedbc`` files.
  - Minor fixes and improvements

- 2020-12-07: 4.0

  - :doc:`concurrency`: a new mechanism to allow multiple distributed processes
    to access the same remote database.
  - File sharing now works in Windows (one process can read a file while
    another is writing it, but two processes cannot open the same file for
    writing).
  - :ref:`joedb_embed` compiles the content of a database into a C++ string
    literal.
  - support for generating code inside a nested namespace (``namespace
    deeply::nested::tutorial`` in the .joedbc file).
  - .deb packages are provided for easy installation.
  - Minor fixes and improvements

- 2019-11-19: 3.0

  - More flexibility for opening files:

    - A database can be based on a C++ stream (which allows compression, encryption, or building a database into an executable as a string).
    - A read-only database can be opened directly from within an Android apk, without having to extract the file first.
    - See :ref:`opening_files` for more details.

  - Better portability:

    - Defining the ``JOEDB_PORTABLE`` macro builds joedb with portable C++ only (no file locking, no fsync). With this option, joedb can be used on the PlayStation 4 and the Nintendo Switch.
    - Unlike in Linux, ``fseek`` and ``ftell`` are 32-bit in Windows. So the previous version could not handle files larger than 2^31 bytes. This is now fixed, and very large files can be used in Windows.
    - Unlike Linux, Windows does no print any information when a program is terminated by an exception. Joedb tools in this version catch all exceptions, and print them before quitting.

  - Main version number incremented because of one minor change: custom functions are now member of ``Generic_File_Database`` instead of the ``File_Database`` class.

  - Minor fixes and improvements.

- 2018-04-02: 2.1

  - new :ref:`joedb_merge` tool to concatenate joedb files
  - dense table storage is more memory-efficient in the interpreter
  - Minor fixes and improvements

- 2017-01-18: 2.0

  - Exceptions everywhere: no more error codes, no more bad states, better diagnostics.
  - Safety: several safety checks were added. This version was thoroughly fuzzed, and should not crash on any input file. Many assertions were added to detect data-manipulation errors (double delete, double insert, reading invalid rows, etc.).
  - Better handling of read-only files and locking. A file opened for writing can now be opened for reading by other processes. Readers won't be updated by changes made by the writer, but it is still more convenient than before.
  - The compiler can produce a rudimentary C wrapper around the C++ classes.
  - :ref:`joedb_to_json`
  - Tested on big-endian and 32-bit machines
  - Many minor fixes and improvements

- 2016-11-18: 1.0
