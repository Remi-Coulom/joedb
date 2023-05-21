TODO
====

Journal File
------------
- joedb_truncate <file> <position> (+optionally show position in logdump)
- Add an ``undo`` operation to the log. This way, it is possible to keep all
  branches of history.
- joedb_fix
- Test (and don't allow) file size > 2Gb in 32-bit code (in theory, should also
  test if 64-bit overflows).
- When opening a journal: ignore-error and set-checkpoint-to-file-size should
  be two independent options. Must have a "robust" option that will silently
  ignore incomplete transactions (or the whole file?) for automatic fail-safe
  operation. Must not ignore test for checkpoint integrity when opening
  read-only. Create a "shared_read" opening mode that disables writing (but
  re-enables it after reading the checkpoint).

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

Blobs
-----
- Cheaper than SHA 256: sample 256*4kb pages from the file
- Optimized full SHA 256:

  - sha256 log entry
  - store "incrementable" SHA 256
  - linked list: store position of previous sha256 log entry + NSA shortcuts

- show progress bars for slow operations
- interactive database browser (with labels + filters)
- network protocol extension to handle local blob cache without downloading everything

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

- Lock objects (file + connection). Make file unlocking nothrow? That would simplify a lot.
- Test many concurrent read and write requests. Performance benchmarks.
- File-locking benchmarks

- joedb_server:

  - use coroutines
  - support running on multiple threads (requires mutex?)

    - OK to keep one thread busy when waiting for a lock, or computing SHA 256, ...
    - thread_count = max(core_count, 2 * server_count)
    - Requires synchronization. Mutex for global stuff (connection, disconnection, interrupt, ...)

  - use a journal-only client instead of directly manipulating a journal
  - indicate commit level for a push
  - allow timeout in the middle of a push.
  - don't use a big push buffer. Push to the file directly?
  - fuzzer + unit testing

- performance: fuse socket writes. Fused operations can be produced by fusing
  writes. Lock-pull and push-unlock could have be done this way.
- Notifications from server to client, in a second channel:

  - when another client makes a push
  - when the lock times out
  - when the server is interrupted
  - ping

- Readonly_Client, Readonly_Server
- server: get rid of signal completely. It is really ugly. Make an interactive command-line interface to control the server.
- ipv6 server: https://raw.githubusercontent.com/boostcon/2011_presentations/master/wed/IPv6.pdf
- reading and writing buffers: don't use network_integers.h, but create a
  Buffer_File class, and use write<int64_t>
- Connection_Multiplexer for multiple parallel backup servers?

C++ language questions
----------------------

- Pass strings by value for new and update

  - fix useless copies
  - need to fix Writable + joedbc (it is a bit complicated)
  - start by testing copy elision on a very simple toy simulation
  - necessary to std::move or not?
  - is the compiler allowed to perform the optimization by itself, even if
    the function is passed a const reference?

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
