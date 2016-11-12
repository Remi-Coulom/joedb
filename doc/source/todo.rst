TODO
====

Short term
----------

- documentation
- interpreter: meta commands use ".quit", ".dump", ... , and add ".help"
- db.null_<table>() to get the null pointer
- error management:

  - When replaying the journal:

    - more explicit error explanation
    - put the journal in bad state

  - When using the compiled code:

    - write a time stamp and a message in the log
    - avoid crashing (no throw)

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
- joedb_fix
- Compression

New operations and types
------------------------
- Use diff for large-string update

On-disk storage
----------------
- sqlite
- stxxl? For strings: store a big vector of chars. A string is length + index in the big vector of chars.

Compiler
--------
- store each field as a vector (probably not so important)
- modularize code generation
- Table options:

  - single_row: compiled to a simple struct, with simpler getters.

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
- index returned by public methods of Freedom_Keeper should be like a std::vector (start at zero, don't count used_list and free_list).
- vim syntax and completer with YouCompleteMe
- GUI editor similar to the icga database editor (fastcgi, interpreter)
- rapidly undo-able history
- add explicit keyword to constructors
- make some classes non-copyable
