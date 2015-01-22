TODO
====

Short term
----------
- make simple compiler
- schema-only option for replay_log

Interpreter
-----------
- coverage tests for JournalFile etc.
- string format/escaping 'string with space' ...
- readline, help
- dump journal to interpreter commands
- sql dump -> sqlite3
- dump to compact new .joedb file

- optimize data structures (they are simple but inefficient)

Journal file
------------
- detect endianness at compile time for faster io
- high-performance system-specific implementation of joedb::File, with fsync (asynchronous?), custom buffers, ...
- more compact record insertion (record_id + all values at the same time)

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

- simple compiler

.. code-block:: c++

    class database;

    class person
    {
      friend class database;
      private:
        record_id_t id;
        person(record_id_t id): id(id) {} // only the database can construct
      public:
        record_id_t get_record_id() const {return id;}
    };

    struct person_data
    {
      std::string name;
      db_namespace::city city;
    }

- core compiler options:

  * namespace as parameter
  * mutex protection as option
  * triggers: C++ code: after/before insert/update/delete

- utility compilation (with triggers): vector cache for a table.

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
