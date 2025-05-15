TODO
====

For next release
----------------

 - Improvements:

   - joedb::Construction_Flags in constructor of Writable_Client as well
   - joedbc:

     - unique indexes:

       - encapsulate multi-column update (cannot write column individually)
       - do not allow column to be in more than one unique index
       - find_or_new_<index>(cols)
       - delete_<index>(cols)
       - update_<index>(id, cols)
       - in case of unique index failure, throw before actually inserting
       - use struct (with field names) instead of tuple for index key

     - option to make some member functions private (->private: private_new_person)
     - option to add custom member functions

   - joedbc_fuzzer must work without debug assertions: check input in release mode as well

     - replace JOEDB_ASSERT by JOEDB_RELEASE_ASSERT in compiled code
     - more efficient test for validity of a range of ids for vector insert/update/delete

 - Tooling:

   - Add support for vcpkg
   - FetchContent and find_package
   - vscode syntax highlighting: https://code.visualstudio.com/api/language-extensions/syntax-highlight-guide
   - Use clang-format (try to customize it, use tabs)

On-disk Storage
---------------
- In a directory
- A checkpoint file (2 copies, valid if identical)
- A subdirectory for each table
- One file per column vector
- One file for string data (string column = size + start_index)
- Use memory-mapped files (llfio)
- SQLite Writable?

Compiler
--------
- Differentiate between "storage type" and "usage type":

  - remove bool type and use int8 instead, with bool usage
  - usages: bool(int8), date(int64).
  - uint8, uint16, uint32, uint64
  - custom usage label: ip address(int32), URL(string), PNG file(string),
    UTF8(string) (use base64 instead for json output), ...?

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

- Table options:

  - no_delete: allows more efficient indexing (+smaller code)
  - last N (for web access log) (last 0 = none)

- Allow the user to write custom event-processing functions and store
  information in custom data structures (for instance: collect statistics from
  web access log without storing whole log in RAM).
- Compiler utilities:

  - referential integrity
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

Concurrency
-----------
- asynchronous hard checkpoint: add wait_for_hard_checkpoint(int64_t) function
  in Connection.
- joedb_server:

  - use coroutines
  - stress-test tool
  - support running on multiple threads (requires mutex?)

    - OK to keep one thread busy when waiting for a lock, or computing SHA 256, ...
    - thread_count = max(core_count, 2 * server_count)
    - Requires synchronization. Mutex for global stuff (connection, disconnection, interrupt, ...)

  - support for log rotation: https://stackoverflow.com/questions/53188731/logging-compatibly-with-logrotate
  - write log as joedb file?

- Do not crash on write error, continue to allow reading?
- Asynchronous client code:

  - Robust_Connection to synchronous backup should not block reads in Server
  - Connection_Multiplexer for multiple parallel synchronous backup servers
  - it is difficult to do in practice, because libssh is not very asynchronous
  - best way might be to use threads

Use case: log with safe real-time remote backup
-----------------------------------------------

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

Performance
-----------

- Memory-mapped specialization of Abstract_File using llfio
- use async_write_some and async_read_some during pull and push
- FILE_FLAG_SEQUENTIAL_SCAN or explicit asynchronous prefetch: https://devblogs.microsoft.com/oldnewthing/20221130-00/?p=107505

joedb_admin
-----------
- work as a client to a joedb_server.
- customizable GUI, similar to the icga database editor.

Other Ideas
-----------
- Android logcat (custom std::streambuf, not part of joedb)
- One separate class for each exception, like ``joedb::exception::Out_Of_Date``.
- ability to indicate minimum joedb version in .joedbc file (and .joedbi?)
- better readable interface:

  - a separate table abstraction (that could be used for query output)
  - cursors on tables

- Deal properly with inf and nan everywhere (logdump, joedb_admin, ...)
- Note that SQL does not support inf and nan. Use NULL instead.
- Raw commands in interpreter?
- import from SQL
- joedb_pack: option to fill holes left by deleted elements
