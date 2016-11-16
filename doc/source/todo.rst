TODO
====

Short term
----------

- get rid of db_schema in journal_file, and use separate update for each type
- single cpp that includes all dependencies for linking to compiled code
- add a log event that indicates that data is valid at this point
- runtime error when creating a hole in vector storage (can happen only when replaying the journal)
- error management:

  - When replaying the journal:

    - more explicit error explanation
    - put the journal in bad state
    - throw an exception instead of slow calling of virtual is_good()

  - When using the compiled code:

    - log the event that produced the error
    - log a time stamp and a comment + checkpoint_no_commit()

Interpreter
-----------
- show command prompt
- readline, help
- SQL import

Journal File
------------
- joedb_truncate <file> <position> (+optionally show position in logdump)
- check for write errors (out of space) -> exception (option?)
- high-performance system-specific implementation of joedb::File?
- Try using a raw device (probably requires a big buffer)
- joedb_fix
- Compression

New operations and types
------------------------
- Use diff for large-string update
- Date type?

On-disk Storage
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

Other Ideas
-----------
- index returned by public methods of Freedom_Keeper should be like a std::vector (start at zero, don't count used_list and free_list).
- vim syntax and completer with YouCompleteMe
- GUI editor similar to the icga database editor (fastcgi, interpreter)
- rapidly undo-able history
- add explicit keyword to constructors
- make some classes non-copyable
