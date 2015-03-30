TODO
====

Short term
----------

Compiler:

- check matching db schema when opening file, and allow creating a new file
- check C++ identifier constraints for table and field names
- check namespace != joedb
- find_xxx method (with index)
- custom triggers

Interpreter
-----------
- show command prompt
- readline, help
- sql dump
- use sql syntax
- dump to compact new .joedb file

Journal file
------------
- make sure the same file cannot be opened by two programs at the same time
- check for write errors (out of space) -> exception (option?)
- high-performance system-specific implementation of joedb::File?
- Try using a raw device (probably requires a big buffer)
- Compression

New operations and types
------------------------
- more compact record insertion (record_id + all values at the same time)
- also: insertion without record_id for default append should be more compact
- "single-row" table option, compiled to a simple struct.
- "don't-reuse-deleted-rows" table option
- "no-delete" table option
- checkpoints, tags, etc.
- rename operations (table, field)
- more data types

  - varchar
  - date
  - vector<int>

On-disk storage
---------------
- sqlite: with no fsync...
- stxxl? For strings: store a big vector of chars. A string is length + index in the big vector of chars.

Compiler
--------

- make sure identifiers can't produce other collisions

- core compiler options:

  * mutex protection as option
  * triggers: C++ code: after/before insert/update/delete
  * commit/checkpoint policy:

    - never
    - when flushing write buffer
    - at every log entry or transaction_end
    - commit before/after/before&after checkpoint

- Compiler utilities:

  - table storage:

    - any stl container (vector, deque, map, unordered_map)
    - file (maybe, for big tables): make on-disk C++ containers

  - index, unique constraints (use triggers)
  - referential integrity (use triggers)
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

Other ideas
-----------
- GUI editor similar to the icga database editor (http server with cpp-netlib)
- rapidly undo-able history
- add explicit keyword to constructors
- make some classes non-copyable
