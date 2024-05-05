# Joedb, the Journal-Only Embedded Database

SQL Databases are the standard tool for manipulating relational data with
[ACID](https://en.wikipedia.org/wiki/ACID) transactions, but they are not
convenient to use from C++. Raw SQL APIs such as
[SQLite](https://www.sqlite.org/cintro.html) do not match the C++ programming
paradigm well: queries are strings parsed at run time, and the API is not
type-safe. An
[ORM](https://en.wikipedia.org/wiki/Object%E2%80%93relational_mapping) or Query
Object Model can hide SQL from the C++ code, and improve type safety, but it
can result in sub-optimal performance, and adds to complexity.

In many applications, the immense complexity of a SQL database is not at all
necessary. Joedb is a minimalist low-level alternative that offers the
possibility to:

 - incrementally add data to a file in a crash-safe way,
 - synchronize data access between multiple processes, on the same machine or
   over a network connection,
 - offer a direct low-level C++ API, with compile-time checking of field names
   and types.

As shown in the diagram below, joedb compiles the database schema into C++
code. Applications using this code can then manipulate data like a C++
container.

![Diagram](doc/source/images/joedb.svg)

Other systems such as [protocol buffers](https://protobuf.dev/) provide similar
type-safe mechanisms for serializing structured data, but are missing key
properties of a relational database, such as concurrency, transactions, and
incremental crash-safe updates. Joedb can offer all these features by storing
data as a journal of transactions. The whole data history is stored, so it is
possible to re-create any past state of the database. Joedb also has a network
protocol, and can operate in a distributed fashion, a bit like [git for
structured data](https://www.remi-coulom.fr/joedb/concurrency.html).

To give an order of magnitude of database complexities, the table below shows
the number of lines of source code of joedb, and some SQL databases. These are
without tests, and measured with ``grep -c $``.

|Database  |Lines of code|
|:---------|------------:|
|joedb     |       18,403|
|SQLite    |      240,533|
|PostgreSQL|    1,512,558|

So joedb is an extremely simple low-level foundation for sharing relational
data. It would be possible to build a SQL database on top of it, but it is
already very convenient as-is.

For more information, please take at a look at the
[documentation](https://www.remi-coulom.fr/joedb/intro.html).
