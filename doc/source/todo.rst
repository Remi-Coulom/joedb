TODO
====

Short Term
----------

- compiler safety:

  - check valid record_id for listener updates and deletes
  - #ifndef NDEBUG -> check double delete/insert, etc.
  - max_record_id for listener inserts

- make Readable_Writeable based on compiled db (or Readable only...)

- conan.io

Journal File
------------
- joedb_truncate <file> <position> (+optionally show position in logdump)
- high-performance system-specific implementation of joedb::File?
- make file work independently of machine endianness?
  Determining endianness at compile time is difficult:
  http://stackoverflow.com/questions/4239993/determining-endianness-at-compile-time
- Try using a raw device (probably requires a big buffer)
- joedb_fix
- Compression

New Operations and Types
------------------------
- Use diff for large-string update
- Differentiate between "storage type" and "usage type":

  - remove bool type and use int8 instead, with bool usage
  - usages: bool(int8), date(int64).
  - custom usage label: ip address(int32), URL(string), PNG file(string), UTF8(string) (use base64 instead for json output), ...?

On-disk Storage
----------------
- LevelDB? https://github.com/google/leveldb
- sqlite?
- stxxl? For strings: store a big vector of chars. A string is length + index in the big vector of chars.

Compiler
--------
- use std::set and std::multiset for indexes? Might be better for strings.
- store each field as a vector
- before_update_vector: ask for storage pointer to the listener
- debug code with range checking
- modularize code generation
- Table options:

  - single_row: compiled to a simple struct, with simpler getters.

- Compiler utilities:

  - referential integrity
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

- C wrapper. Catch all exceptions? Error codes?
- jni wrapper

Server
------
- Controls access to a single joedb file.
- Manages history only. Each client manages its own individual table storage.
- Clients can get/release exclusive write access.
- Writes are broadcast to all clients, handled as triggers.

Other Ideas
-----------
- import from SQL
- index returned by public methods of Freedom_Keeper should be like a std::vector (start at zero, don't count used_list and free_list).
- vim syntax and completer with YouCompleteMe
- GUI editor similar to the icga database editor (fastcgi, interpreter)
- rapidly undo-able history
- add explicit keyword to constructors
- make some classes non-copyable
- Use templates instead of virtual function calls for writeables?

  - compilation will be slower
  - compiled code may get bigger if more than one template instance
  - but avoiding virtual calls makes code run faster (and may get smaller)
  - worth it only if measurably faster
