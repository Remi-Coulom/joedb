Indexes
=======

Indexes allow fast finding and sorting at the price of slower data manipulation, and increased memory usage. For every index, the compiler will generate code that automatically updates an ``std::map`` or an ``std::multimap``.

Defining Indexes
----------------

Indexes are defined in the .joedbc configuration file. The syntax is:

.. code-block:: none

  create[_unique]_index <index_name> <table> <field>[,<field>...]

Note that there must be no space around the comma.

For example, the joedb tutorial has these index definitions:

.. literalinclude:: ./tutorial/src/tutorial.joedbc
   :language: joedbc

So, two cities can't have the same name, but two persons can. An update that would create a duplicate city name will throw an exception.

Using Indexes
-------------

The most universal way to use an index is to directly use the const reference to the ``std::map`` or ``std::multimap`` returned by the ``get_index_of_<index_name>`` function of the database. The compiler also generates a convenient ``find_<index_name>`` function for finding rows.

Note: a new entry in the index is created at the first update of the last field
of the index. Uninitialized rows are not entered into the index.

Here is an example with the tutorial database:

.. literalinclude:: ./tutorial/src/index_tutorial.cpp
   :language: c++

And here is its output:

.. literalinclude:: ./tutorial/index_tutorial.out
   :language: none

Handling Unique-Index Errors
----------------------------

If the update of an unique index fails because it would create a duplicate,
then an exception is thrown. The invalid update is not written to the journal,
or the index, but it is stored in memory. If you wish to keep the in-memory
data consistent after such a failure, you must fix the invalid update by
executing a valid one.

Multi-Column Index Update
-------------------------

A multi-column index is updated when the last column of the index is modified.
