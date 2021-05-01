Joedb
=====

Joedb is the Journal-Only Embedded Database. It stores structured data as a
journal of all modifications. This way, the whole data history is remembered,
and it is possible to re-create any past state of the database. It is also a
way to make the system extremely simple, and fast.

Joedb comes with a compiler that takes a database schema as input, and produces
C++ code. The generated C++ data-manipulation code is convenient to use,
efficient, and type-safe.

.. image:: doc/source/images/joedb.svg

For more information, please take at a look at the _`documentation
https://www.remi-coulom.fr/joedb/intro.html`.
