History
=======

- 2017-??-??: 2.0

  - Exceptions everywhere: no more error codes, no more bad states, better diagnostics.
  - Safety: several safety-checks were added. This version was thoroughly fuzzed, and should not crash on any input file. Many assertions were added to detect data-manipulation errors (double delete, double insert, reading invalid rows, etc.).
  - Better handling of read-only files and locking. A file opened for writing can now be opened for reading by other processes.
  - The compiler can produce a C wrapper around the C++ classes
  - ``joedb_to_json``
  - Many minor fixes and improvements

- 2016-11-18: 1.0
