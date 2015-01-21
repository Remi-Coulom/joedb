TODO
====

simple interactive interpreter
------------------------------

#) coverage tests for JournalFile etc.
#) database listener that produces a list of interpreter commands
#) interpreter: string format/escaping 'string with space' ...

simple compiler
---------------

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

- transactions:

  - write "begin", "commit", "rollback" operations to journal
  - support for "rollback" is costly: make it a compiler option
  - truncate journal at load time. If unfinished transaction, append a rollback

- core compiler options:

  * namespace as parameter
  * mutex protection as option
  * triggers: C++ code: after/before insert/update/delete

- utility compilation (with triggers): vector cache for a table.

make it good
------------

- high-performance system-specific implementation of joedb::File, with fsync (asynchronous?), custom buffers, ...
- rename operations (table, field)
- documentation
- compact dump (insert with data instead of field-by-field updates)
- Compiler utilities:

  - table storage:

    - any stl container (vector, deque, map, unordered_map)
    - file (maybe, for big tables): make on-disk C++ containers
    - easy loop over database records (for (auto person: db.persons))

  - index, unique constraints (use triggers)
  - referential integrity (use triggers)
  - queries
  - incrementally-updated group-by queries (OLAP, hypercube, ...)

- more data types: date, ip, vector<int>, ...
- option to have rapidly undo-able history
- schema option: reuse deleted ids (implement with a linked list of free recs)
- don't store only table: also store variables. This can be done with a "single-row" table option. http://stackoverflow.com/questions/2300356/using-a-single-row-configuration-table-in-sql-server-database-bad-idea
- a table can be made "read-only" as an option
- possibility to add tags to the database log, and load data up to the tag.
- GUI editor similar to the icga database editor (http server with cpp-netlib)
