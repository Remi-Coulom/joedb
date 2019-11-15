History
=======

- 2019-11-??: 3.0

  - Better portability:

    - Defining the ``JOEDB_PORTABLE`` macro builds joedb with portable C++ only (no file locking, no fsync). With this option, joedb can be used on the PlayStation 4 and the Nintendo Switch.
    - Unlike in Linux, ``fseek`` and ``ftell`` are 32-bit in Windows. So the previous version could not handle files larger than 2^31 bytes. This is now fixed, and very large files can be used in Windows.
    - Unlike Linux, Windows does no print any information when a program is terminated by an exception. Joedb tools in this version catch all exceptions, and print them before quitting.

  - More flexibility for opening files:

    - A read-only database can be opened directly from within an Android apk.
    - A database can be based on a C++ stream (which allows compression, encryption, or building a database into an executable as a string).
    - See :ref:`opening_files` for more details

  - Main version number incremented because of one minor change: custom functions are now member of `Generic_File_Database` instead of the `File_Database` class;

- 2018-04-02: 2.1

  - new :ref:`joedb_merge` tool to concatenate joedb files
  - dense table storage is more memory-efficient in the interpreter
  - Minor fixes and improvements

- 2017-01-18: 2.0

  - Exceptions everywhere: no more error codes, no more bad states, better diagnostics.
  - Safety: several safety-checks were added. This version was thoroughly fuzzed, and should not crash on any input file. Many assertions were added to detect data-manipulation errors (double delete, double insert, reading invalid rows, etc.).
  - Better handling of read-only files and locking. A file opened for writing can now be opened for reading by other processes. Readers won't be updated by changes made by the writer, but it is still more convenient than before.
  - The compiler can produce a rudimentary C wrapper around the C++ classes.
  - :ref:`joedb_to_json`
  - Tested on big-endian and 32-bit machines
  - Many minor fixes and improvements

- 2016-11-18: 1.0
