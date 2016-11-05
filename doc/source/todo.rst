TODO
====

Short term
----------

- time stamp
- custom error management

- benchmark & profile: memory usage and opening of analysis file
- measure time to produce each page on crazy sensei

- index after update, not after insert (or use default value)
- index-based sorting

- make sure identifiers can't produce collisions

- Table options:

  - vector:

    - no delete
    - insert at the back
    - one std::vector for each field

  - single_row: compiled to a simple struct, with simpler getters.

Interpreter
-----------
- show command prompt
- readline, help
- SQL import

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
- modularize code generation
- Compiler utilities:

  - referential integrity (use triggers)
  - queries (SQL compiler?)
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

- C wrapper (with "EXPORTED_FUNCTIONS", for emscripten)

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
