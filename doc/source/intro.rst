Joedb, the Journal-Only Embedded Database
=========================================

Joedb is a minimalist embedded relational database, where data is manipulated directly in the target programming language, without using SQL: a compiler produces native data-manipulation code from the database schema.

In joedb, the journal of all modifications is stored to disk. This way, the whole data history is remembered, and it is possible to re-create any past state of the database. It is also a way to make the system extremely simple, and fast.

Is Joedb for me?
----------------

Joedb currently has some limitations:

- the database must fit in memory
- only one process can open the database at the same time
- C++ is the only supported programming language

Note that those limitations are likely to disappear in the future.

Joedb offers many nice features that may make it more attractive than typical alternatives like sqlite, xml, or json:

- Since a joedb file is append-only, its crash-safe operation does not require flushing data to disk as frequently as typical relational databases.
- The whole data history is remembered. So, no old data can ever be lost. It is also possible to add time stamps and comments to the journal, and use it as a log of the application.
- If the history has to be forgotten for privacy reasons, it is possible to pack it.
- If the database schema of an application changes over time, joedb can upgrade files to the new version automatically. The upgrade includes changes to the schema as well as custom data manipulation.
- The database schema is compiled into C++ code that allows convenient type-safe data manipulation. Many errors that would be detected at run time with SQL, xml, or json will be detected at compile time instead.
- Joedb is very simple, light, and fast.

What is it Like to Use Joedb?
-----------------------------

The database schema is described in a file like this one:

.. literalinclude:: ./tutorial/tutorial.joedbi

Compiler instructions are in a separate file:

.. literalinclude:: ./tutorial/tutorial.joedbc

Running the compiler with these files will generate tutorial.h and tutorial.cpp, two files that you can use to manipulate data conveniently in C++:

.. literalinclude:: ./tutorial/tutorial_main.cpp
   :language: c++

Running this tutorial will produce this output:

.. literalinclude:: ./tutorial/tutorial.out

All the data was stored in the tutorial.joedb file. The database file is a binary file, so it is not convenient to inspect it directly. The joedb_logdump tool will produce a readable log:

.. literalinclude:: ./tutorial/logdump.out

What's Next?
------------
