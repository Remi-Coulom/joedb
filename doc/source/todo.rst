TODO
====

short term (before review):

- Rewrite Freedom Keeper
- Silence outputs of Server and Server_Connection

Journal File
------------
- Hashing for hardware error detection.
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
  - uint8, uint16, uint32, uint64
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
- check that vector range is OK in constructor of vector update
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
  - last N (for web access log) (last 0 = none)

- Allow the user to write custom event-processing functions and store
  information in custom data structures (for instance: collect statistics from
  web access log without storing whole log in RAM).
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
- joedb_to_json can also become more efficient
- Get ready for "last-N" storage, and no_delete option (force single interval).

Concurrency
-----------
- joedb_server:

  - fuzzer + unit testing
  - use coroutines
  - option to serve readonly
  - indicate commit level for a push
  - cache SHA-256 calculations + efficient incremental update.
  - perform hash calculations asynchronously (don't block whole server)

- joedb_client:

  - make it asynchronous instead of using threads

- Connection via domain sockets / named pipes
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

C++ language questions
----------------------

- Pass strings by value for new and update

  - fix useless copies
  - need to fix Writable + joedbc (it is a bit complicated)
  - start by testing copy elision on a very simple toy simulation
  - necessary to std::move or not?
  - is the compiler allowed to perform the optimization by itself, even if
    the function is passed a const reference?

- Is it undefined behavior to make a copy of size zero past the end?

  - in ``joedb/journal/Memory_File.h``. MSVC complains in debug mode.
  - pessimistic: https://stackoverflow.com/questions/29844298/is-it-legal-to-call-memcpy-with-zero-length-on-a-pointer-just-past-the-end-of-an
  - optimistic: https://en.cppreference.com/w/cpp/string/byte/memcpy
  - by the way, we should decide whether its is legal for joedb vectors as
    well. Do like C++.

Performance
-----------

- vector of size 1: write ordinary insert and update to the journal instead
- joedb::Database: use vector instead of map for tables and fields (with a bool
  indicating if deleted)

joedb_admin
-----------
- serve with boost::beast.
- work as a client to a joedb_server.
- customizable GUI, similar to the icga database editor.

Other Ideas
-----------
- Is it possible to replace macros by templates?
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
