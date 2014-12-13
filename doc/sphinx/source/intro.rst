Introduction
============

crazydb is a relational database that stores data in an append-only journal file. It is designed to be used as an embedded database. It comes with a compiler that takes a database schema as input and produces C++ code. This code can load the data from the journal file into transparently persistent C++ data structures.

This approach allows crazydb to have the following nice properties:

- The full data history is stored. It is possible to undo any data modification.
- There is no SQL string manipulation. This removes the possibility of SQL injection, and the performance cost of SQL interpretation.
- Tables of the relational schema are compiled into classes. This way, errors in tables names, field names or types can be detected at compile time, instead of run time.
- This approach offers great opportunities for performance optimization. Data is directly stored in C++ data structures. The programmer is free to choose which container best fits each table. So, for instance, a dense table can be kept in a simple C++ std::vector if it is small enough to fit in memory. Huge tables can be stored on disk with `stxxl <http://stxxl.sourceforge.net/>`_ containers. Indexes and triggers are implemented in C++, too. So, in short, the programmer of the application controls the data-management code completely, and can make it as efficient as possible.

The drawbacks of this approach, compared to more traditional database management systems are:

- It is necessary to read the whole data history when opening a crazydb file. This may be slow for large amounts of data.
- The crazydb file may become very big compared to the size of the data for a database that has a lot of updates and deletes.

In many situations, these drawbacks are not a problem. For a server application that is rarely restarted, a slow startup time is not so annoying.  And if it is really a problem to keep the whole history (for space or privacy reasons), then the journal file can be compacted to keep only the most recent state.

5-minute tutorial
-----------------

The application can manipulate the data this way

.. code-block:: c++

    #include "tutorial.h"

    int main()
    {
      tutorial::database db("tutorial.crazydb");

      tutorial::city Lille = db.insert_city();
      db.update_name(Lille, "Lille");

      tutorial::person remi = db.insert_person("Rémi", tutorial::city::null);

      db.update_city(remi, Lille);

      db.delete_person(remi);

      db.begin_transaction();
      db.end_transaction();

      return 0;
    }

Benchmark
---------

Comparison with Other Systems
-----------------------------
