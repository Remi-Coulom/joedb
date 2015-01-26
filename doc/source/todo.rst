TODO
====

Short term
----------
Finish interpreter data structures:

- doubly linked list for free and used table records (like before)
- template for update/get by type?
- make sure the database interface is fail-safe, but also make sure that table/field/record existence is not checked more than once.
- don't check too much in interpreter
- make sure big field data structure is not copied
- make sure fields of deleted records are reset to default

Compiler:

- implement Listener methods to fill vectors
- implement data-manipulation methods
- check matching db schema when opening file
- joedbc_insert benchmark

Interpreter
-----------
- string format/escaping 'string with space' ...
- show command prompt
- readline, help
- dump journal to interpreter commands
- sql dump
- use sql syntax
- dump to compact new .joedb file

Journal file
------------
- detect overflow in compact_write?
- high-performance system-specific implementation of joedb::File?
- Try using a raw device (probably requires a big buffer)

New operations and types
------------------------
- "single-row" table option, compiled to a simple struct.
- "don't-reuse-deleted-rows" table option
- "no-delete" table option
- checkpoints, tags, etc.
- rename operations (table, field)
- more compact record insertion (record_id + all values at the same time)
- more data types

  - varchar
  - date
  - vector<int>

Compiler
--------

- check C++ identifier constraints for table and field names

- core compiler options:

  * namespace as parameter
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
    - easy loop over database records (for (auto person: db.persons))

  - index, unique constraints (use triggers)
  - referential integrity (use triggers)
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

Other ideas
-----------
- GUI editor similar to the icga database editor (http server with cpp-netlib)
- rapidly undo-able history
- add explicit keyword to constructors
