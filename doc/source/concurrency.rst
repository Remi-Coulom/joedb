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
exception that merging branches is not possible. History must be linear, and a
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
process. It is not very useful, except for unit testing, and illustrating
concurrency for this tutorial.

``Local_Connection``
^^^^^^^^^^^^^^^^^^^^

``Local_Connection`` allows serverless concurrent connection to a file. It is a
template that takes a file class as parameter. It works only with file classes
that support locking, such as ``Posix_File`` and ``Windows_File`` (or ``File``,
when it is typedefed to one of those).

Here is an example of use:

.. literalinclude:: ./tutorial/local_concurrency.cpp
   :language: c++

Running this program multiple times illustrates some aspects of error
management in joedb. The first run will create the database with the the 3
cities, and complete without errors. Because of the unique index, the second
run will fail:

.. literalinclude:: ./tutorial/local_concurrency_2.txt

This leaves the database with an incomplete transaction, which will prevent
opening the file in the third run:

.. literalinclude:: ./tutorial/local_concurrency_3.txt

You can observe the content of the aborted transaction using ``joedb_logdump --ignore-errors``:

.. literalinclude:: ./tutorial/local_concurrency.joedbi
   :language: joedbi

The situation can be resolved by using :ref:`joedb_convert` to produce a fixed
file. The ``--ignore-errors`` flag can be used to include the aborted
transaction into the output.

``Server_Connection``
^^^^^^^^^^^^^^^^^^^^^

``Server_Connection`` allows connecting to a running :ref:`joedb_server` using
the joedb :doc:`network protocol <network_protocol>`.

The constructor of ``Server_Connection`` takes a ``Channel`` parameter. Two
channel classes are provided:

 * ``Network_Channel`` opens a network socket to the server directly.
 * ``ssh::Forward_Channel`` connects to the server with ssh encryption and authentication.

The source code of :ref:`joedb_ssh_client` shows an example of use:

.. literalinclude:: ../../src/joedb/io/joedb_ssh_client.cpp
   :language: c++
