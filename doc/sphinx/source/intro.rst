Introduction
============

crazydb is an embedded relational database that stores data in an append-only journal file. It comes with a compiler that takes a database schema as input and produces C++ code. This code can load the data from the journal file into transparently persistent C++ data structures.

Context and Motivation
----------------------

The most basic approach to writing programs with persistent data stored in a relational database consists in using SQL queries via some API. Most database management systems provide such an API. Generic interfaces such as ODBC and JDBC are available, too.

This basic SQL approach has many problems. Writing a program that produces a SQL string at run time is dangerous: it is a source of security risks because of SQL injection. It also has a cost in terms of performance, because the SQL has to be parsed and interpreted by the database. Also, many errors that should be detected at compile time, such as a typo in the name of a field, will be detected at run time.

Some weaknesses of the basic SQL approach can be corrected by encapsulating the dirty business of crafting SQL strings into some higher-level interfaces such as object-relational mapping systems or data-access objects. These system improve safety by providing static typing and identifier lookup.

These abstract systems might look much cleaner from the programmer's point of view, but the additional layer of abstraction often has some cost, such as abstraction inversion. Abstraction inversion is when the programmer of the application may end-up having to re-implement a feature that was hidden by the abstraction. For example, it might become necessary to use a loop over objects to update them one by one. One single complex SQL query might have done the job efficiently, but the abstraction forces the programmer to inefficiently generate several queries instead.

The idea of crazydb is to overcome these problems by dropping SQL, and all the abstraction layers. All the operations over the relational data are directly implemented in the target programming language. This produces an architecture that is considerably cleaner and simpler.

In this minimalist approach, crazydb stores data in permanent storage with an append-only journal. In order to implement transactions, and to make the system robust to crashes, writing a journal is necessary. And because the journal can contain all the data, it is not necessary to make anything else permanent.

A journal is a minimal representation of the database, but, for many typical operations, it is not a convenient representation. In practice, an application that uses crazydb will build in-memory or on-disk tabular structures and indexes to manipulate the data conveniently. But these are nothing more than a temporary redundant cache: the only official source of data is the journal. When a crazydb database is opened again, the tabular structures are rebuilt from the journal.

Pros and Cons
-------------

Pros:

- There is no SQL string manipulation. This removes the possibility of SQL injection, and the performance cost of SQL interpretation.
- Tables of the relational schema are compiled into classes. This way, errors in tables names, field names or types can be detected at compile time, instead of run time.
- The full data history is stored in crazydb. In many applications based on traditional relational database management systems, database designers end up having to create history tables in order to keep data history. Typically, these table are filled by triggers. This is another example of ineffective abstraction inversion. Because any database management system has to write a journal anyway, the revision history should be stored there.
- This approach offers great opportunities for performance optimization. Data is directly stored in C++ data structures. The programmer is free to choose which container best fits each table. So, for instance, a dense table can be kept in a simple C++ std::vector if it is small enough to fit in memory. Huge tables can be stored on disk with `stxxl <http://stxxl.sourceforge.net/>`_ containers. Indexes and triggers are implemented in C++, too. So, in short, the programmer of the application controls the data-management code completely, and can make it as efficient as possible.

Cons:

- It is necessary to read the whole data history when opening a crazydb file. This may be slow for large amounts of data. For a server application that is rarely restarted, a slow startup time is not a big problem. But if the application has to be rapidly responsive at startup, and the database is big, then crazydb might not be a good choice.
- The crazydb file may become very big compared to the size of the data for a database that has a lot of updates and deletes. If necessary, it is possible to compact the journal file and keep only the most recent state. But this operation is not done transparently and may be slow.
- crazydb is an embedded database: it does not have the flexibility of the traditional client/server architecture where multiple separately-programmed clients can connect to the same database server. crazydb can be used inside a server, but the programmer has to implement the server logic.
- The database schema is statically determined at compile-time, and cannot change during the execution of a compiled application. The schema of a crazydb file can be modified by a separate tool, and schema-modifications operations are logged in the crazydb file. But such schema modifications cannot occur during the execution of a compiled application.

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
