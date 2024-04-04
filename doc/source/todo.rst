TODO
====

Short-term fixes for next release
---------------------------------

- Server option, either:

  - Lock the connection during the whole life of the server, shallow pulls
  - Lock the connection only when locking the server, deep pulls

- Allow timeout during push
- Check::all & ~Check::big_size
- Thoroughly test server (timeout during push, interrupted push, ...). Must be
  automated unit tests.
- Connection_Multiplexer for multiple parallel backup servers
- Witty joedb_admin + joedb for kifusnap training set before release

Journal File
------------
- Allow opening compiled database with Check::all & ~Check::big_size
- FILE_FLAG_SEQUENTIAL_SCAN or explicit asynchronous prefetech: https://devblogs.microsoft.com/oldnewthing/20221130-00/?p=107505
- Test (and don't allow) file size > 2Gb in 32-bit code (in theory, should also test if 64-bit overflows).

New Operations and Types
------------------------
- Add an ``undo`` operation to the log. This way, it is possible to keep all
  branches of history.
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
- Pull-only connection (eg when serving a read-only file):
  -> joedb_client does not offer transaction and push
  -> reply with readonly flag during server handshake
  -> bool is_pullonly() const in connection (and client)
- joedb_server:

  - Test many concurrent read and write requests. Performance benchmarks.
  - fuzzer + unit testing
  - use coroutines
  - support running on multiple threads (requires mutex?)

    - OK to keep one thread busy when waiting for a lock, or computing SHA 256, ...
    - thread_count = max(core_count, 2 * server_count)
    - Requires synchronization. Mutex for global stuff (connection, disconnection, interrupt, ...)

  - allow timeout in the middle of a push.
  - ipv6: https://raw.githubusercontent.com/boostcon/2011_presentations/master/wed/IPv6.pdf
  - get rid of signal. Make an interactive command-line interface to control
    the server. Maybe better: use asio's (non-std::net) support for signal.

- SHA-256: option for either fast or full.
- performance: fuse socket writes (TCP_NODELAY, TCP_QUICKACK). Fused operations
  can be produced by fusing writes. Lock-pull and push-unlock could have been
  done this way. https://www.extrahop.com/company/blog/2016/tcp-nodelay-nagle-quickack-best-practices/
- Lock objects (file + connection) necessary for joedb_admin? Make file unlocking nothrow? That would simplify a lot.
- reading and writing buffers: don't use network_integers.h, but create a
  Buffer_File class, and use write<int64_t>
- Notifications from server to client, in a second channel:

  - when another client makes a push
  - when the lock times out
  - when the server is interrupted
  - ping

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
- One separate class for each exception, like ``joedb::exception::Out_Of_Date``.
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

- compiled Readable
- index and referential integrity: should be in the journal, and also
  implemented in the interpreted database?
- Deal properly with inf and nan everywhere (logdump, joedb_admin, ...)
- Note that SQL does not support inf and nan. Use NULL instead.
- Raw commands in interpreter?
- import from SQL
- rapidly undo-able history?
