.. _concurrency:

Concurrency
===========

Joedb offers mechanisms to access a single database from multiple processes.

Principle
---------

Concurrency works by letting each process have a local copy of the central
database. Each process can keep data synchronized with 3 basic operations:

- **pull**: update local data with new journal entries from the central database.
- **lock_pull**: get exclusive write access to the central database. The lock operation may have to wait for another process to release its lock first. Also update local data.
- **push_unlock**: update the central database with the local modifications, and release the lock. This works only if the database is currently locked by a previous **lock_pull**.

Joedb uses RAII to ensure locks and unlocks are correctly paired, and
modifications to the local database can only occur during a lock.

Example C++ Code
----------------

.. literalinclude:: ./tutorial/concurrency_tutorial.cpp
   :language: c++

It produces this output:

.. literalinclude:: ./tutorial/concurrency_tutorial.out
   :language: none

Connections
-----------

The constructor of the ``tutorial::Client`` class takes two parameters: a
connection, and a file for local storage. The connection is an object of the
``joedb::Connection`` class, that provides the 3 basic operations described
above. This section presents the different kinds of available connections.

``Embedded_Connection``
^^^^^^^^^^^^^^^^^^^^^^^

This will create a connection to a file opened in the same program. It does not
allow concurrent access to the same file. This class is convenient to allow the
same polymorphic code to work either with a shared remote database or a local
file that is not shared.

``SSH_Connection``
^^^^^^^^^^^^^^^^^^

This allows sharing a file stored on a remote machine via ssh. There is no need
to run a joedb server on the remote machine. Locking works with a mutex file,
named with ``.mutex`` appended to the name of the database file. The same mutex
file is used to ensure the atomicity of the pull operation, so reads are
blocked when one client has a write lock.

There is no mechanism to deal with crash or disconnection of a client holding
the lock. The mutex file will remain on the server, and it will keep blocking.
If this happens, you'll have to fix the situation manually. If a disconnection
occurred in the middle of a big push, then the server database might be
incomplete. It should be fixed (for instance, by copying the database of the
client that disconnected) before removing the mutex.

If you need better performance or reliability, use the joedb server instead.

``SSH_Robust_Connection``
^^^^^^^^^^^^^^^^^^^^^^^^^

This is a wrapper around ``SSH_Connection`` that will try to reconnect whenever
an exception is thrown.

``Server_Connection``
^^^^^^^^^^^^^^^^^^^^^

TODO. Will be much more efficient and reliable than ``SSH_Connection``.
