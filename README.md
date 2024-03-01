# Joedb, the Journal-Only Embedded Database

The traditional approach to [ACID](https://en.wikipedia.org/wiki/ACID) storage
of structured data consists in using a SQL database, but using SQL from <nobr>C++</nobr> is
not very convenient. Raw SQL APIs such as
[SQLite](https://www.sqlite.org/cintro.html) are very complex to use, and do
not match the C++ programming paradigm well: queries are strings parsed at run
time, and the API is not type-safe. An
[ORM](https://en.wikipedia.org/wiki/Object%E2%80%93relational_mapping) can help
to hide SQL from the C++ code, and improve type safety, but it adds to
complexity, and can result in sub-optimal performance. SQL is meant to be a
high-level user interface to data, and does not match the low-level nature of
C++ programming.

Joedb aims to offer an alternative to using SQL from C++, by providing a direct
low-level access to data. As shown in the diagram below, the database schema is
compiled into C++ code, and can be manipulated like a C++ container.

![Diagram](doc/source/images/joedb.svg)

Other systems such as [protocol buffers](https://protobuf.dev/) offer similar
type-safe mechanisms for serializing structured data, but are missing key
properties of a SQL database, such as concurrency, transactions, and
incremental crash-safe update of data. Joedb can offer all these features by
storing data as a journal of transactions. The whole data history is stored, so
it is possible to re-create any past state of the database. Joedb has a network
protocol, and can operate in a distributed fashion, a bit like [git for
structured data](https://www.remi-coulom.fr/joedb/concurrency.html).

For more information, please take at a look at the
[documentation](https://www.remi-coulom.fr/joedb/intro.html).
