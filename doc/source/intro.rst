Introduction
============

Joedb stands for the "Journal-Only Embedded Database". It is a light-weight
C++ database that keeps tabular data in memory, and writes a journal to a
file. The whole data history is stored, so it is possible to re-create any
past state of the database. Joedb has a network protocol, and can operate in a
distributed fashion, a bit like :doc:`git for data <concurrency>`. It provides
`ACID <https://en.wikipedia.org/wiki/ACID>`_ transactions for local and remote
concurrent access to a file.

Joedb comes with a compiler that takes a database schema as input, and produces
C++ code. The generated C++ data-manipulation code is convenient to use,
efficient, and type-safe.

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
  incrementally update data stored on disk in a crash-safe way, and can handle
  :doc:`concurrent connections <concurrency>` of multiple clients to a single
  database.
- The whole data history is stored. So, no old data can ever be lost. It is
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
  This may change with support of on-disk data storage. Also, :ref:`blobs
  <blobs>` allow manipulating databases that are much bigger than available
  RAM.
- C++ is the only supported programming language.

Compared to history-less databases, joedb has one fundamental drawback:
frequently-updated values may make the joedb journal file grow very large.

So joedb might not be the best choice for every situation, but it is great if
data fits in RAM, has to be stored safely on disk, and is manipulated by C++
code.

An Example
----------

A simple example of how to use joedb is available in the
``doc/source/tutorial/src`` directory. The database schema is defined in the
``tutorial.joedbi`` file:

.. literalinclude:: ./tutorial/src/tutorial.joedbi
   :language: joedbi

Compiler instructions are in ``tutorial.joedbc``:

.. literalinclude:: ./tutorial/src/tutorial.joedbc
   :language: joedbc

This tutorial database can be compiled into C++ source code with :ref:`joedbc`:

.. code-block:: bash

    joedbc tutorial.joedbi tutorial.joedbc

This will produce various source files in the ``tutorial`` directory, that can
be used to manipulate data conveniently in C++, as shown in the
``tutorial_main.cpp`` source file:

.. literalinclude:: ./tutorial/src/tutorial_main.cpp
   :language: c++

Running the resulting program will produce this output:

.. literalinclude:: ./tutorial/tutorial.out
   :language: none

All the data was stored in the tutorial.joedb file. The database file is a
binary file, so it is not convenient to inspect it directly. The
:ref:`joedb_logdump` tool can produce a readable log:

.. literalinclude:: ./tutorial/logdump.joedbi
   :language: joedbi

Concurrency Examples
--------------------

The example in the previous section opens a joedb database directly, and
prevents concurrent writes. In order to share write access to the database with
other processes, it is also possible to use write transactions with a
:joedb:`Client` instead:

.. literalinclude:: ./tutorial/src/hello_concurrency.cpp
   :language: c++

All writes to the database are done via this transaction function, which
automatically handles locking, checkpointing, and unlocking. A joedb client can
also connect to a remote database via the joedb network protocol. For more
details, check the :doc:`concurrency` section of the User Guide.

The subsections below illustrate concurrency features using joedb's interactive
command-line tools.

Concurrent Access to a Local File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Write transactions to the same file with a joedb client are mutually exclusive:

.. asciinema:: ./asciinema/local_concurrency_interpreted.cast
   :poster: npt:0:01
   :speed: 1.5

Concurrent Access to a Remote Server
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. asciinema:: ./asciinema/remote_concurrency.cast
   :poster: npt:0:01
   :speed: 1.5

Asynchronous Real-Time Backup
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The joedb server can notify a client as soon as new data is available, so that
a client can perform backups without polling.

.. asciinema:: ./asciinema/asynchronous_backup.cast
   :poster: npt:0:01
   :speed: 1.5

Synchronous Real-Time Backup
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With synchronous real-time backup, the client has to wait for the data to be
written to the backup before a transaction can complete. So it is safer than
asynchronous backup, at the cost of latency.

.. asciinema:: ./asciinema/synchronous_backup.cast
   :poster: npt:0:01
   :speed: 1.5
