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
  and release the lock. This works only if the database is currently locked by
  a previous **lock_pull**.

Example C++ Code
----------------

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
``joedb::Connection`` class, that provides the 3 synchronization operations
(pull, lock_pull, push_unlock). This section presents the different kinds of
available connections.

``Embedded_Connection``
^^^^^^^^^^^^^^^^^^^^^^^

``Embedded_Connection`` creates a connection to a file opened in the same
program. It does not allow concurrent access to the same file from another
process. This class is convenient to allow the same polymorphic code to work
either with a shared remote database or a local file that is not shared.

``Local_Connection``
^^^^^^^^^^^^^^^^^^^^

``Local_Connection`` allows serverless concurrent connection to a local file
from multiple processes running on the same machine. It is a template that
takes a file class as parameter. It works only with file classes that support
locking, such as ``Posix_File`` and ``Windows_File`` (or ``File``, when it is
typedefed to one of those). The file parameter of the client constructor must
be the value returned by the ``get_file()`` method.

The source code of :ref:`joedb_local_client` shows an example of use:

.. literalinclude:: ../../src/joedb/io/joedb_local_client.cpp
   :language: c++

``Server_Connection``
^^^^^^^^^^^^^^^^^^^^^

``Server_Connection`` allows connecting to a running :ref:`joedb_server` using the joedb :doc:`network protocol <network_protocol>`.

The constructor of ``Server_Connection`` takes a ``joedb::Channel`` parameter. Two channel classes are provided:

 * ``joedb::Network_Channel`` opens a network socket to the server directly.
 * ``joedb::ssh::Forward_Channel`` connects to the server with ssh encryption and authentication.

The source code of :ref:`joedb_ssh_client` shows an example of use:

.. literalinclude:: ../../src/joedb/io/joedb_ssh_client.cpp
   :language: c++
