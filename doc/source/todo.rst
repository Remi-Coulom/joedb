TODO
====

Journal File
------------
- joedb_truncate <file> <position> (+optionally show position in logdump)
- better than truncating: add an ``undo`` operation to the log. This way, it is possible to keep all branches of history.
- joedb_fix
- CRC for hardware error detection
- Test (and don't allow) file size > 2Gb in 32-bit code

New Operations and Types
------------------------
- Use diff for large-string update
- Differentiate between "storage type" and "usage type":

  - remove bool type and use int8 instead, with bool usage
  - usages: bool(int8), date(int64).
  - custom usage label: ip address(int32), URL(string), PNG file(string), UTF8(string) (use base64 instead for json output), ...?

On-disk Storage
----------------
- LevelDB: https://github.com/google/leveldb

Compiler
--------
- An uninitialized field is undefined. Debug mode: check that no undefined value is read? Also: an index may have more than one field. Should not include a row into the index before all fields of the index are defined? Only one index columns triggers index update?
- modularize code generation

  - Each module should have:

    - required include files
    - data structure for storing data
    - additional hidden table fields?
    - triggers (after/before insert/update/delete)
    - public methods

  - Possible to modularize:

    - indexes
    - sort functions
    - referential integrity
    - safety checks
    - incrementally updated group-by queries

- use std::set and std::multiset for indexes? Might be better for strings.
- Table options:

  - single_row: compiled to a simple struct, with simpler getters.

- Compiler utilities:

  - referential integrity
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

- C wrapper. Catch all exceptions? Error codes?
- jni wrapper

Concurrency
-----------
- Make a server (boost::asio or networking TS).
- Make the system robust with respect to client disconnections during write

  - set a timeout: clients not allowed to hold a lock for too long.
  - push_unlock may fail if the lock timed out, and the checkpoint advanced on
    the server.

- Optional CRC check before pulling to verify the content of the origin.
- joedb::Readonly_Connection (pull only).

Other Ideas
-----------
- ability to indicate minimum joedb version in joedbc (and joedbi?)

- apply schema upgrade to readonly databases (custom functions)

- tests for buffer overflow could be optimized when reading/writing large
  pieces of data, such as strings or vectors.

- optimize vector update: read file directly into the column thanks to get_own_storage.

- only one file.check_write_buffer() call in write<T> and compact_write<T>:
  make code shorter and simpler.

- Pass strings by value for new and update

  - fix useless copies
  - need to fix Writable + joedbc (it is a bit complicated)
  - start by testing copy elision on a very simple toy simulation
  - method for testing: use a very large string (100Mb) + pause execution with sleep + look at process memory usage. (also measure execution time).
  - main question: necessary to std::move or not?

- make a package for vcpkg and conan. Maybe build2?

- Unique index over multiple columns should work. Needs a way to modify multiple columns atomically. New operation: start/end atomic record update. Also new operation: insert_and_start_atomic_update. Make it work also for vector insertions.
- Null default initial values
- better readable interface:

  - a separate table abstraction (that could be used for query output)
  - cursors on tables

- make Readable_Writable based on compiled db (or Readable only...)
- make joedb_admin work on the new readable interface, and publish it
- index and referential integrity: should be in the journal, and also implemented in the interpreted database.
- performance of interpreted database: use vector instead of map for tables and fields (with a bool indicating if deleted)

- Deal properly with inf and nan everywhere (logdump, joedb_admin, ...)
- Raw commands in interpreter?
- import from SQL
- index returned by public methods of Freedom_Keeper should be like a std::vector (start at zero, don't count used_list and free_list).
- vim syntax and completer with YouCompleteMe
- GUI editor similar to the icga database editor (fastcgi, interpreter)
- rapidly undo-able history
- add explicit keyword to constructors
- make some classes non-copyable
- Use templates instead of virtual function calls for writables?

  - compilation will be slower
  - compiled code may get bigger if more than one template instance
  - but avoiding virtual calls makes code run faster (and may get smaller)
  - worth it only if measurably faster
