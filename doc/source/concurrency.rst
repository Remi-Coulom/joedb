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

``Embedded_Connection``
^^^^^^^^^^^^^^^^^^^^^^^

``Embedded_Connection`` creates a connection to a file opened in the same
program. It does not allow concurrent access to the same file from another
process. It is not very useful in practice, except for unit testing, and this
tutorial.

``Local_Connection``
^^^^^^^^^^^^^^^^^^^^

``Local_Connection`` allows serverless concurrent connection to a file, using file-locking for synchronization.

:ref:`joedbc <joedbc>` produces a convenient ``Local_Client`` class that creates the file and the client in a single line of code. Here is an example:

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

The example code below shows how to connect to a server via ssh:

.. literalinclude:: ../../src/joedb/concurrency/SSH_Server_Connection.h
   :language: c++
