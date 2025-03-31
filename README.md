# Joedb, the Journal-Only Embedded Database

Joedb is a minimalist in-memory database, that allows directly manipulating
tables stored in native type-safe C++ containers. It comes with a server and a
network protocol that makes sharing data between machines as easy as sharing
variables between threads. Joedb also allows writing data to files with proper
crash-safe concurrent [ACID](https://en.wikipedia.org/wiki/ACID) transactions.

The diagram below illustrates how the system works: `joedbc`, the joedb
compiler, reads the database schema and produces classes that can be used to
manipulate files:

![Diagram](doc/source/images/joedb.svg)

Other systems such as [protocol buffers](https://protobuf.dev/) provide similar
type-safe mechanisms for serializing structured data, but are missing key
properties of a database, such as concurrency, transactions, and incremental
crash-safe updates. Joedb can offer all these features by storing data as a
journal of transactions. The whole data history is stored, so it is possible to
re-create any past state of the database. Joedb also has a network protocol,
and can operate a bit like [git for structured
data](https://www.remi-coulom.fr/joedb/concurrency.html).

To give an indication of joedb's simplicity, the table below shows the size of
joedb's source code compared to some other libraries. Lines of code are counted
with ``wc -l``. Compressed size is the size in bytes after removing comments
and compressing with gzip. These numbers were measured on 2024-07-28 on the
main branch.

| Software       | Lines of code | Compressed size | Notes                             |
|:---------------|--------------:|----------------:|:----------------------------------|
| joedb          |        19,288 |          72,268 | without tests                     |
| nlhohmann/json |        24,941 |          87,168 | single include                    |
| boost/json     |        37,609 |         123,229 |                                   |
| SQLite         |       258,281 |       1,067,983 | sqlite3.c amalgamation            |
| PostgreSQL     |     1,513,329 |       5,250,316 | content of src dir, without tests |

So joedb is an extremely simple low-level foundation for sharing relational
data. It would be possible to build a SQL database on top of it, but it is
already very convenient by itself.

For more information, please take at a look at the
[documentation](https://www.joedb.org/).
