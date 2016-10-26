TODO
====

Short term
----------
Compiler:

- custom data-upgrade code when necessary (new log entry: "custom <name>")
- new log entry: rename table/field
- flush data before throwing
- periodic flush to system / periodic sync ?
- new log entry: comment
- new log entry: time stamp
- better checkpoint types:
   * checkpoint_no_commit
   * checkpoint_half_commit
   * checkpoint_full_commit

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
- checkpoints, tags, etc.
- rename operations (table, field)

- more data types

  * varchar<N>
  * date
  * vector<T>

On-disk storage
----------------

- sqlite
- stxxl? For strings: store a big vector of chars. A string is length + index in the big vector of chars.

Compiler
--------

- "single-row" table option, compiled to a simple struct, with simpler getters.
- "no-delete" table option

- make sure identifiers can't produce other collisions
- custom triggers, modularize code generation
- index-based sorting

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
