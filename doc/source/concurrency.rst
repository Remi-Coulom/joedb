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

Joedb uses RAII to ensure locks and unlocks are correctly paired, and
modifications to the local database can only occur during a lock.

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

This will create a connection to a file opened in the same program. It does not
allow concurrent access to the same file from another process. This class is
convenient to allow the same polymorphic code to work either with a shared
remote database or a local file that is not shared.

``SSH_Connection``
^^^^^^^^^^^^^^^^^^

This allows sharing a file stored on a remote machine via ssh. There is no need
to run a joedb server on the remote machine. Locking works with a mutex file,
named with ``.mutex`` appended to the name of the database file. The same mutex
file is used to ensure the atomicity of the pull operation, so reads are
blocked when one client has a write lock.

There is no mechanism to deal with crash or disconnection of a client holding
the lock. The mutex file will remain on the server, and it will keep blocking.
If this happens, the situation has to be fixed manually. If a disconnection
occurred in the middle of a big push, then the server database might be
incomplete. It should be fixed (for instance, by copying the database of the
client that disconnected) before removing the mutex.

For better performance or reliability, use the :ref:`joedb server <joedb_server>` instead.

``Server_Connection``
^^^^^^^^^^^^^^^^^^^^^

``Server_Connection`` allows connecting to a running :ref:`joedb_server`. The
server can use a timeout for the lock, and allows simultaneous pulls by many
clients, so it should offer much better performance and reliability than
``SSH_Connection``.
