TODO
====

For next release
----------------
 - use joedb to store a struct (mahjong rules, training parameters, ...)

   - struct defined only once (in joedbi format, with default values)
     (add_field table int32 default 1234)
   - joedbc generates all C++ code for convenient manipulation
   - single-row compiler option

 - proper handling of unique_index with more than one column:

   - joedbc produces a function to update multiple values simultaneously. Index
     columns cannot be updated individually.
   - do not allow more than one unique index with the same last column.
   - when reading the file, update index only when last column is updated
   - This may break old files.

 - deletion of field used in custom function

 - File specialization that stores content as history of buffer writes. For each buffer:

   - position
   - size
   - data (blob) (can be compressed. pass a codec class)

 - log rotation, ability to delete or compress early part of the log:

   - multi-part file
   - keeps a table with all parts
   - keep first part as schema definition + checkpoint
   - skip deleted parts when reading
   - option to compress a part at rotation time

 - Asynchronous Server Connection (for tamper-proof log backup)

   - does not wait for confirmation after push
   - can batch frequent pushes (do not send new push until after receiving the previous push confirmation)
   - keeps working even if server dies

 - Add support for vcpkg and conan

New Operations and Types
------------------------
- Add an ``undo`` operation to the log. This way, it is possible to keep all
  branches of history.

- Use diff for large-string update
- Differentiate between "storage type" and "usage type":

  - remove bool type and use int8 instead, with bool usage
  - usages: bool(int8), date(int64).
  - uint8, uint16, uint32, uint64
  - custom usage label: ip address(int32), URL(string), PNG file(string),
    UTF8(string) (use base64 instead for json output), ...?

Blobs
-----
- network protocol extension to handle local blob cache without downloading everything
- zero-copy access to blob data using memory-mapped file

On-disk Storage
---------------
- In a directory
- A checkpoint file (2 copies, valid if identical)
- A subdirectory for each table
- One file per column vector
- One file for string data (string column = size + start_index)
- Use memory-mapped files (is there a portable way?)

Compiler
--------
- Pass strings by value for new and update, and std::move them:

  - need for rvalue reference overload of Writable::update_string
  - plain reference version must be kept as well
  - using blobs or vectors of int8 can be a high-performance alternative
  - so for the moment, it is not worth the added complexity

- allow reading dropped fields in custom functions that are invoked before the
  drop. Store data in a column vector, and clear the vector at the time of the
  drop. Make sure field id is not reused. (make access function private, and
  custom functions are friends)
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
- A linked list of intervals instead, to unify everything
- Let joedb_merge fuse intervals to remove holes (100% update_vector)
- joedb_to_json can also become more efficient
- Get ready for "last-N" storage, and no_delete option (force single interval).

Concurrency
-----------
- joedb_server:

  - fuzzer
  - use coroutines
  - stress-test tool
  - support running on multiple threads (requires mutex?)

    - OK to keep one thread busy when waiting for a lock, or computing SHA 256, ...
    - thread_count = max(core_count, 2 * server_count)
    - Requires synchronization. Mutex for global stuff (connection, disconnection, interrupt, ...)

  - ipv6: https://raw.githubusercontent.com/boostcon/2011_presentations/master/wed/IPv6.pdf
  - get rid of signal. Make an interactive command-line interface to control
    the server. Maybe better: use asio's (non-std::net) support for signal.

- SHA-256: option for either none, fast or full.
- Connection_Multiplexer for multiple parallel backup servers? Complicated.
  requires asynchronous client code.
- Do not crash on write error, continue to allow reading?
- Notifications from server to client, in a second channel:

  - when another client makes a push
  - when the lock times out
  - when the server is interrupted
  - ping

Performance
-----------

- File design based on llfio
- use async_write_some and async_read_some during pull and push
- vector of size 1: write ordinary insert and update to the journal instead
- joedb::Database: use vector instead of map for tables and fields (with a bool
  indicating if deleted)
- FILE_FLAG_SEQUENTIAL_SCAN or explicit asynchronous prefetch: https://devblogs.microsoft.com/oldnewthing/20221130-00/?p=107505

joedb_admin
-----------
- serve with boost::beast.
- work as a client to a joedb_server.
- customizable GUI, similar to the icga database editor.

Other Ideas
-----------
- One separate class for each exception, like ``joedb::exception::Out_Of_Date``.
- Is it possible to replace macros by templates?
- ability to indicate minimum joedb version in joedbc (and joedbi?)
- apply schema upgrade to readonly databases (custom functions)
- Null default initial values
- better readable interface:

  - a separate table abstraction (that could be used for query output)
  - cursors on tables

- compiled Readable
- index and referential integrity: should be in the journal, and also
  implemented in the interpreted database?
- Deal properly with inf and nan everywhere (logdump, joedb_admin, ...)
- Note that SQL does not support inf and nan. Use NULL instead.
- Raw commands in interpreter?
- import from SQL
- rapidly undo-able history?
- namespace for each subdir?
