TODO
====

Short Term
----------

- better readable interface:

  - a separate table abstraction (that could be used for query output)
  - cursors on tables

- make Readable_Writeable based on compiled db (or Readable only...)
- make joedb_admin work on the new readable interface, and publish it
- index and referential integrity: should be in the journal, and also implemented in the interpreted database.
- performance of interpreted database: use vector instead of map for tables and fields (with a bool indicating if deleted)

Journal File
------------
- Instead of throwing an exception: make it impossible at compile time to open a writeable journal with a read_only file.
- joedb_truncate <file> <position> (+optionally show position in logdump)
- high-performance system-specific implementation of joedb::File?
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
- LevelDB: https://github.com/google/leveldb

Compiler
--------
- Allow opening a compiled db in "create_new" mode to throw if existing?
- Should inheritance of compiled db from Writeable be protected?
- An un-initialized field is undefined. Debug mode: check that no undefined value is read? Also: an index may have more than one field. Should not include a row into the index before all fields of the index are defined? Only one index columns triggers index update?
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
- store each field as a vector
- before_update_vector: ask for storage pointer to the listener
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
- Raw commands in interpreter?
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
