Joedb
=====

Joedb is the Journal-Only Embedded Database. Joedb stores structured data as a
journal of all modifications. This way, the whole data history is remembered,
and it is possible to re-create any past state of the database. It is also a
way to make the system extremely simple, and fast.

Storing data history also allows efficient distributed applications. Joedb has
a network protocol that lets multiple processes access shared data
concurrently. It works a bit like git for structured data (without merging).

.. image:: doc/source/images/joedb.svg

For more information, please take at a look at the _`documentation
https://www.remi-coulom.fr/joedb/intro.html`.
