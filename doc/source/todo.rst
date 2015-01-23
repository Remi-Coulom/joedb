TODO
====

Short term
----------
Compiler:

- storage vectors: linked lists like in interpreter
- implement Listener methods to fill vectors
- implement data-manipulation methods
- check matching db schema when opening file
- joedbc_insert benchmark

Interpreter
-----------
- Keep highest record_id for easy table append
- coverage tests for JournalFile etc.
- string format/escaping 'string with space' ...
- show command prompt
- readline, help
- dump journal to interpreter commands
- sql dump -> sqlite3
- dump to compact new .joedb file

- optimize data structures (they are simple but inefficient)

Journal file
------------
- detect endianness at compile time for faster io
- dynamically adjust the number of bytes for field_id_t, table_id_t, record_id_t? Also string size?
- high-performance system-specific implementation of joedb::File, with fsync (asynchronous?), custom buffers, ...
- more compact record insertion (record_id + all values at the same time)
- crash resistance: need fsync before and after write in the checkpoint region.
- Try using a raw device

New operations and types
------------------------
- "single-row" table option, compiled to a simple struct.
- checkpoints, tags, etc.
- rename operations (table, field)
- more data types

  - varchar
  - date
  - vector<int>

Compiler
--------

- core compiler options:

  * namespace as parameter
  * mutex protection as option
  * triggers: C++ code: after/before insert/update/delete

- Compiler utilities:

  - table storage:

    - any stl container (vector, deque, map, unordered_map)
    - file (maybe, for big tables): make on-disk C++ containers
    - easy loop over database records (for (auto person: db.persons))

  - index, unique constraints (use triggers)
  - referential integrity (use triggers)
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

Other ideas
-----------
- GUI editor similar to the icga database editor (http server with cpp-netlib)
- rapidly undo-able history
