TODO
====

Journal File
------------
- Hashing for hardware error detection, and checking that client.joedb matches
  server.joedb before pulling. Git is transitioning to SHA-256, so this may be
  a good choice. Find a code that can be easily updated incrementally when
  appending data.
- joedb_truncate <file> <position> (+optionally show position in logdump)
- better than truncating: add an ``undo`` operation to the log. This way, it is
  possible to keep all branches of history.
- joedb_fix
- Test (and don't allow) file size > 2Gb in 32-bit code

New Operations and Types
------------------------
- Needs a way to modify multiple columns atomically (allows unique_index to
  work + better trigger invocations). New operations:

  - start/end atomic record update
  - insert_and_start_atomic_update.
  - Also for vector insertions and updates.

- Use diff for large-string update
- Differentiate between "storage type" and "usage type":

  - remove bool type and use int8 instead, with bool usage
  - usages: bool(int8), date(int64).
  - custom usage label: ip address(int32), URL(string), PNG file(string),
    UTF8(string) (use base64 instead for json output), ...?

On-disk Storage
----------------
- In a directory
- A checkpoint file (2 copies, valid if identical)
- A subdirectory for each table
- One file per column vector
- One file for string data (string column = size + start_index)
- Use memory-mapped files (is there a portable way?)

Compiler
--------
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
  - no_delete: allows more efficient indexing (+smaller code)
<<<<<<< HEAD
  - last N (for web access log) (last 0 = none)
=======
  - set_table_storage last N (for web access log) (last 0 = none)
>>>>>>> eea404b3d4af717130940fdf01f677200bb90c1c

- Compiler utilities:

  - referential integrity
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

- C wrapper. Catch all exceptions? Error codes?
- jni wrapper

Better Freedom_Keeper
---------------------
- index returned by public methods of Freedom_Keeper should be record ids.
- No need to maintain a linked list of individual records
- A linked list of intervals instead, to unify everything?
- Let joedb_merge fuse intervals to remove holes (100% update_vector)
- Get ready for "last-N" storage, and no_delete option (force single interval).

Concurrency
-----------
- joedb_server:

  - fuzzer + unit testing
  - use coroutines
  - event listener instead of direct writes to std::cerr
  - indicate checkpoint type for each push (commit or no_commit)

- Optional CRC check before pulling to verify the content of the origin.
- Shared_Local_File: don't copy the file to memory. Create a File_Continuation
  class that takes a readonly and a writable file, and uses the writable file
  for the header and the continuation: only the header has to be copied.
- File_Continuation can be used to create a branch (and cancel transaction in
  case of failure to push).
- performance: merge socket writes.
- Notifications from server to client, in a second channel:

  - when another client makes a push
  - when the lock times out
  - when the server is interrupted
  - ping

Performance
-----------

- vector of size 1: write ordinary insert and update to the journal instead

- interpreted database

  - use vector instead of map for tables and fields (with a bool indicating if deleted)

- Pass strings by value for new and update (or use C++17 string_view?)

  - fix useless copies
  - need to fix Writable + joedbc (it is a bit complicated)
  - start by testing copy elision on a very simple toy simulation
  - method for testing: use a very large string (100Mb) + pause execution with
    sleep + look at process memory usage. (also measure execution time).
  - necessary to std::move or not?

- Use templates instead of virtual function calls for writables?

  - compilation will be slower
  - compiled code may get bigger if more than one template instance
  - but avoiding virtual calls makes code run faster (and may get smaller)
  - worth it only if measurably faster

joedb_admin
-----------
 - serve with boost::beast.
 - work as a client to a joedb_server.
 - customizable GUI, similar to the icga database editor.

Other Ideas
-----------
- joedb_merge and joedb_pack should remove holes, and produce compact vectors.
- ability to indicate minimum joedb version in joedbc (and joedbi?)
- apply schema upgrade to readonly databases (custom functions)
- only one file.check_write_buffer() call in write<T> and compact_write<T>:
  make code shorter and simpler.
- make a package for vcpkg and conan. Maybe build2?
- Null default initial values
- better readable interface:

  - a separate table abstraction (that could be used for query output)
  - cursors on tables

- make Readable_Writable based on compiled db (or Readable only...)
- index and referential integrity: should be in the journal, and also
  implemented in the interpreted database?
- Deal properly with inf and nan everywhere (logdump, joedb_admin, ...)
- Note that SQL does not support inf and nan. Use NULL instead.
- Raw commands in interpreter?
- import from SQL
- GUI editor similar to the icga database editor (fastcgi, interpreter)
- rapidly undo-able history?
- add explicit keyword to constructors
- make some classes non-copyable
