TODO
====

Short term
----------
- index-based sorting
- id-based sorting
- make sure identifiers can't produce collisions
- Table options:

  - vector:

    - insert/delete at the back only
    - store as std::vector of struct
    - return const std::vector<> & to user

  - single_row: compiled to a simple struct, with simpler getters.

- Compact "vector-insertion" log entry (no new event?)
- C wrapper (with "EXPORTED_FUNCTIONS", for emscripten)

Interpreter
-----------
- show command prompt
- readline, help
- sql dump
- use sql syntax
- dump to compact new .joedb file

Journal file
------------
- check for write errors (out of space) -> exception (option?)
- high-performance system-specific implementation of joedb::File?
- Try using a raw device (probably requires a big buffer)
- Compression

New operations and types
------------------------

On-disk storage
----------------
- sqlite
- stxxl? For strings: store a big vector of chars. A string is length + index in the big vector of chars.

Compiler
--------
- custom error management
- modularize code generation
- Compiler utilities:

  - referential integrity (use triggers)
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

Server
------
- Controls access to a single joedb file.
- Manages history only. Each client manages its own individual table storage.
- Clients can get/release exclusive write access.
- Writes are broadcast to all clients, handled as triggers.

Other ideas
-----------
- vim syntax and completer with YouCompleteMe
- GUI editor similar to the icga database editor (fastcgi, interpreter)
- rapidly undo-able history
- add explicit keyword to constructors
- make some classes non-copyable
