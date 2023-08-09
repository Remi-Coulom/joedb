# Joedb, the Journal-Only Embedded Database

Joedb is a light-weight C++ relational database. Joedb keeps tabular data in
memory, and writes a journal to a file. The whole data history is stored, so it
is possible to re-create any past state of the database. Joedb has a network
protocol, and can operate in a distributed fashion, a bit like [git for
structured data](https://www.remi-coulom.fr/joedb/concurrency.html).

Joedb comes with a compiler that takes a database schema as input, and produces
C++ code. The generated C++ data-manipulation code is convenient to use,
efficient, and type-safe.

![Diagram](doc/source/images/joedb.svg)

For more information, please take at a look at the [documentation](https://www.remi-coulom.fr/joedb/intro.html).
