TODO
====

- logging:
   - rename logger -> log
   - replace all writes to cerr by logging in parsers
   - replace progress bars by log messages (x of y done every 3 seconds + estimate remaining time)
   - remove Progress_Bar, create a Progress_Logger class instead
   - Default_System_Logger -> std_System_Logger
   - don't include it twice in Windows
- testing of user-interface code -> (test for joedb_push to file bug)
- always exit cleanly and rapidly after SIGINT or SIGTERM
- channel read timeout should be settable

Notifications
-------------

- Create a "Compiled_Writable" with virtual functions such as "delete_from_city, ..."
- A Compiled_Multiplexer to handle a collection of Compiled_Writables
- Implement default storage with such a Compiled_Writable
- Allow custom data-structure construction
- This will allow:

   - implementing a server to get notifications when a part of the database has changed
   - rollback implementation (easily reset table storage when replaying from scratch)

Stored Procedures
-----------------

- ping thread
- allow keeping a database for duration of a session: if "session.joedbi" exists,
  keep it in memory in the client and server.
- allow implementing and compiling multiple different rpc services on the same database

On-disk Storage
---------------

- Simplest (but inefficient) solution: SQLite
- Home-made efficient solution:

  - In a directory
  - A checkpoint file (2 copies, valid if identical)
  - A subdirectory for each table
  - One file per column vector
  - One file for string data (string column = size + start_index)
  - Use memory-mapped files (llfio)

Compiler
--------

- error if namespace different from file name
- string setters that takes a string_view as parameter
- option to make some member functions private (->private: private_new_person)
- option to add custom member functions
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

  - polymorphic socket: tcp/ip, unix, or websocket
  - stress-test tool
  - support running on multiple threads
  - support for log rotation: https://stackoverflow.com/questions/53188731/logging-compatibly-with-logrotate
  - write log as joedb file?

- Do not crash on write error, continue to allow reading?
- Asynchronous client code:

  - Robust_Connection to synchronous backup should not block reads in Server
  - Connection_Multiplexer for multiple parallel synchronous backup servers
  - it is difficult to do in practice, because libssh is not very asynchronous
  - best way might be to use threads

Use Case: Log with Safe Real-Time Remote Backup
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

- Use C++17 std::map::extract for more efficient index update
- use reference to string instead of copy in index key
- Memory-mapped specialization of Abstract_File using llfio
- use async_write_some and async_read_some during pull and push
- FILE_FLAG_SEQUENTIAL_SCAN or explicit asynchronous prefetch: https://devblogs.microsoft.com/oldnewthing/20221130-00/?p=107505
- Remove one useless round-trip in the constructor of Readonly_Client (empty
  push_and_keep_locked). This operation could be fused with hash checking. Also
  fail if the connection is ahead of the file.

joedb_admin
-----------

- work as a client to a joedb_server.
- customizable GUI, similar to the icga database editor.

Other Ideas
-----------

- Use clang-format (try to customize it, use tabs)
- One separate class for each exception, like ``joedb::exception::Out_Of_Date``.
- ability to indicate minimum joedb version in .joedbc file (and .joedbi?)
- better readable interface:

  - a separate table abstraction (that could be used for query output)
  - cursors on tables

- Deal properly with inf and nan everywhere (logdump, joedb_admin, ...)
- Note that SQL does not support inf and nan. Use NULL instead.
- joedb_pack: option to fill holes left by deleted elements
