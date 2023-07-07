.. _concurrency:

Concurrency
===========

Joedb offers mechanisms to access a single database from multiple processes on
the same machine, or from remote machines over the network.

Principle
---------

Concurrency works by letting each process have a local copy of the central
database. Each process can keep data synchronized with 3 operations:

- **pull**: update local data with new journal entries from the central
  database.
- **lock_pull**: get exclusive write access to the central database, and update
  local data.
- **push_unlock**: update the central database with the local modifications,
  and release the lock.

So it works a bit like `git <https://git-scm.com/>`_, with the significant
difference that merging branches is not possible. History must be linear, and a
global mutex is used to prevent branches from diverging.

When using a remote network connection, a local copy of the database can be
kept in permanent storage between connections. `SHA-256
<https://en.wikipedia.org/wiki/SHA-2>`_ is used at connection time, to check
that the contents of the local and remote copies match. Offline modifications
to the local copy can be made, and then pushed to the server when connecting
later.

Example
-------

The compiler produces code that ensures that locks and unlocks are correctly
paired, and modifications to the local database can only occur during a lock.

.. literalinclude:: ./tutorial/concurrency_tutorial.cpp
   :language: c++

It produces this output:

.. literalinclude:: ./tutorial/concurrency_tutorial.out
   :language: none

Connections
-----------

The constructor of the ``tutorial::Client`` class takes two parameters: a
connection, and a file for local storage. The connection is an object of the
``Connection`` class, that provides the 3 synchronization operations
(pull, lock_pull, push_unlock). This section presents the different kinds of
available connections.

Plain ``Connection``
^^^^^^^^^^^^^^^^^^^^

The ``Connection`` superclass has empty functions for the synchronization
operations, so it does not connect to anything. It can be useful to create a
client with such a connection, because it allows generic code that takes a
client as parameter to work the same way with either a remote connection or a
local file.

``File_Connection``
^^^^^^^^^^^^^^^^^^^

``File_Connection`` creates a connection to a server file opened in the
same program. It does not allow concurrent access to the server file from
another process.

``Readonly_File_Connection``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``Readonly_File_Connection`` creates a connection to a read-only file. It
allows concurrent writes to the file it is connecting to, but cannot lock or
push to it.

``Local_Connection``
^^^^^^^^^^^^^^^^^^^^

Like plain ``Connection``, ``Local_Connection`` does not connect to anything.
But, unlike ``Connection``, it can handle a shared local file, using file
locking for synchronization of multiple processes that may be writing to the
same file.

:ref:`joedbc <joedbc>` produces a convenient ``Local_Client`` class that
creates the connection and the client in a single line of code. Here is an
example:

.. literalinclude:: ./tutorial/local_concurrency.cpp
   :language: c++

Multiple instances of this program can safely write to the same database
concurrently.

``Server_Connection``
^^^^^^^^^^^^^^^^^^^^^

``Server_Connection`` allows connecting to a running :ref:`joedb_server` using
the joedb :doc:`network protocol <network_protocol>`.

The constructor of ``Server_Connection`` takes a ``Channel`` parameter. Two
channel classes are provided:

 * ``Network_Channel`` opens a network socket to the server directly.
 * ``ssh::Forward_Channel`` connects to the server with ssh encryption and authentication.

The code below shows how to connect to a server via ssh:

.. literalinclude:: ../../src/joedb/concurrency/SSH_Server_Connection.h
   :language: c++


.. _local_and_remote_concurrency:

Combining Local and Remote Concurrency
--------------------------------------

A client is made of two parts: the local part (stored in a file), and the
connection part. A client can handle concurrency for both parts simultaneously.
That is to say, it is possible for two different clients to share a connection
to the same remote server, and also share the same local file.

The table below summarizes all available connections.

  +------------------------------+--------------+-----------------+-------------------+
  | Connection Class             | Shared Local | Exclusive Local | Connects to       |
  +==============================+==============+=================+===================+
  | ``Connection``               |              | ✔               | nothing           |
  +------------------------------+--------------+-----------------+-------------------+
  | ``Local_Connection``         | ✔            |                 | nothing           |
  +------------------------------+--------------+-----------------+-------------------+
  | ``File_Connection``          |              | ✔               | an exclusive file |
  +------------------------------+--------------+-----------------+-------------------+
  | ``Readonly_File_Connection`` |              | ✔               | a read-only file  |
  +------------------------------+--------------+-----------------+-------------------+
  | ``Server_Connection``        | ✔            | ✔               | a server          |
  +------------------------------+--------------+-----------------+-------------------+

An exception will be thrown when creating a client if the mode of the file does
not match the mode that the connection supports. Shared local files are
:ref:`opened <opening_files>` with the ``Open_Mode::shared_write`` mode. All
other write modes are exclusive.

.. _backup_client:

Backup Client
-------------

When opening a database with joedb, the journal is read to load table data into
memory. This is an expensive operation that is not necessary when creating a
client whose only purpose is to store a backup replica of a remote server. In
order to avoid this cost, :ref:`joedb_client <joedb_client>` supports a
``--nodb`` option that will not load table data. The ``pull_every`` command can
then be used for efficient periodic backups.
