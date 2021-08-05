Introduction
============

Joedb is the Journal-Only Embedded Database. Joedb stores structured data as a
journal of all modifications. This way, the whole data history is remembered,
and it is possible to re-create any past state of the database. It is also a
way to make the system extremely simple, and fast.

Joedb comes with a compiler that takes a database schema as input, and produces
C++ code. The generated C++ data-manipulation code is convenient to use,
efficient, and type-safe.

Joedb can operate as an embedded database, but also has a server that supports
:doc:`concurrent access to a database <concurrency>`.

.. image:: images/joedb.*

Pros and Cons
-------------

Joedb offers many nice features that may make it more attractive than typical
alternatives such as
`Protocol Buffers <https://developers.google.com/protocol-buffers>`_,
`FlatBuffers <https://google.github.io/flatbuffers/>`_,
`SQLite <https://www.sqlite.org/index.html>`_,
`XML <https://en.wikipedia.org/wiki/XML>`_,
or `JSON <https://www.json.org/json-en.html>`_:

- Unlike XML or JSON, joedb is a binary file format that does not require any
  parsing. So, joedb files are much smaller, and processing data is much
  faster. Joedb also comes with a text format that can be easily read or
  modified by humans, if necessary.
- Unlike Protocol Buffers or FlatBuffers, joedb works like a database: it can
  incrementally update data stored on disk in a crash-safe way, and has a
  network protocol to handle :doc:`concurrent connections <concurrency>` of
  multiple clients to a database server.
- Since a joedb file is append-only, its crash-safe operation does not require
  flushing data to disk as frequently as typical relational databases, which
  can make it an order of magnitude faster (see :doc:`checkpoints` for details
  and benchmarks).
- The whole data history is remembered. So, no old data can ever be lost. It is
  also possible to add time stamps and comments to the journal, and use it as a
  log of the application (if the history has to be forgotten for privacy or
  disk-space reasons, it is also possible to :ref:`pack <joedb_pack>` it).
- If the database schema of an application changes over time, joedb can upgrade
  old files to the new version automatically. The upgrade includes changes to
  the schema as well as custom data manipulation (see :doc:`schema_upgrade`).
- The database schema is compiled into C++ code that allows convenient
  type-safe data manipulation. Many errors that would be detected at run time
  with SQL, XML, or JSON will be detected at compile time instead.
- Joedb is very simple, light, and fast.

Joedb currently has some limitations that may be removed with future
improvements:

- The database is stored in memory. So it must be small enough to fit in RAM,
  and the full journal has to be replayed from scratch when opening a file.
  This may change with support of on-disk data storage.
- C++ is the only supported programming language. A rudimentary C wrapper is
  available.

Compared to history-less databases, joedb has one fundamental drawback:
frequently-updated values may make the joedb journal file grow very large.

So joedb might not be the best choice for every situation, but it is great if
data fits in RAM, has to be stored safely on disk, and is manipulated by C++
code.

An Example
----------

A simple example of how to use joedb is available in the
``doc/source/tutorial`` directory. The database schema is defined in the
``tutorial.joedbi`` file:

.. literalinclude:: ./tutorial/tutorial.joedbi
   :language: joedbi

Compiler instructions are in ``tutorial.joedbc``:

.. literalinclude:: ./tutorial/tutorial.joedbc
   :language: joedbc

This tutorial database can be compiled into C++ source code with :ref:`joedbc`:

.. code-block:: bash

    joedbc tutorial.joedbi tutorial.joedbc

This will produce ``tutorial.h`` and ``tutorial.cpp``, two files that can be
used to manipulate data conveniently in C++, as shown in the
``tutorial_main.cpp`` source file:

.. literalinclude:: ./tutorial/tutorial_main.cpp
   :language: c++

This program can be compiled with this command:

.. code-block:: bash

    c++ -o tutorial tutorial_main.cpp tutorial.cpp -ljoedb

Running the resulting program will produce this output:

.. literalinclude:: ./tutorial/tutorial.out
   :language: none

All the data was stored in the tutorial.joedb file. The database file is a
binary file, so it is not convenient to inspect it directly. The
:ref:`joedb_logdump` tool will produce a readable log:

.. literalinclude:: ./tutorial/logdump.joedbi
   :language: joedbi
