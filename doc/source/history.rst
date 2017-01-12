History
=======

- 2017-??-??: 2.0

  - The compiler can produce a C wrapper around the C++ classes
  - ``joedb_to_json`` tool to export joedb data in json format
  - This version was thoroughly fuzzed, and should not crash on any input. Version 1.0 could crash on malicious input.
  - Exceptions: no more error codes, no more bad states, better diagnostics.
  - Better handling of read-only files. Lock the file for writing only.
  - Many minor fixes and improvements

- 2016-11-18: 1.0
