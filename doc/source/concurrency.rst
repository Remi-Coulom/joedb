.. _concurrency:

Concurrency
===========

Joedb offers mechanisms to access a single database from multiple processes on
the same machine, or from remote machines over the network.

Principle
---------

A joedb client is made of two parts: a file, and a connection. The file stores
a replica of the database, and the connection is used for synchronization. A
connection provides 3 operations:

- **pull**: update the file with new journal entries from the connection.
- **lock_pull**: get exclusive write access to the connection, and pull.
- **push_unlock**: write new modifications of the file to the connection, and release the lock.

So it works a bit like `git <https://git-scm.com/>`_, with the significant
difference that merging branches is not possible. History must be linear, and a
central mutex is used to prevent branches from diverging.

Locking the central mutex is not strictly necessary: offline changes can be
made to the local database without any synchronization with the remote server.
It will still be possible to push those changes to the server when connecting
later, but the push will succeed only if there is no conflict with any other
write that may have occurred.

Example
-------

The compiler produces code that ensures that locks and unlocks are correctly
paired, and modifications to the local database can only occur during a lock.
This is done with transaction function that takes a lambda as parameter, and
executes it between a lock-pull and a push-unlock.

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

The ``Connection`` superclass  does not connect to anything.

One use of this class is to allows generic code that takes a client as
parameter to work the same way with either a remote connection or a local file.

Even though the ``Connection`` superclass does not connect to anything, a
client using this connection still handles concurrent accesses to a file opened
with ``Open_Mode::shared_write``. So this connection can be used to create a
client that handles concurrent accesses to the same file by multiple processes
running on the same machine.

:ref:`joedbc <joedbc>` produces a convenient ``Local_Client`` class that
creates the connection and the client in a single line of code. Here is an
example:

.. literalinclude:: ./tutorial/local_concurrency.cpp
   :language: c++

Multiple instances of this program can safely write to the same database
concurrently.

..
   TODO: ascinema animation

``File_Connection``
^^^^^^^^^^^^^^^^^^^

``File_Connection`` creates a connection to a file. Here are some typical use
cases:

 - ``File_Connection`` can be used to make a safe and clean copy of a database
   that is being used or contains a dirty uncheckpointed transaction.
 - ``File_Connection`` can be used to convert between different file formats.
   For instance, pushing a plain joedb file to a brotli ``Encoded_File`` will
   create a compressed database.
 - A ``File_Connection`` to an ``SFTP_File`` can be a convenient way to pull
   from a remote database without running a joedb server on the remote machine.
   Performance will be inferior to running a joedb server, though. Similarly, a
   ``File_Connection`` to a ``CURL_File`` can be used to pull from a joedb
   database served by a web server.
 - A ``File_Connection`` to a ``Memory_File`` can be used to write clean unit
   tests that do not write to any actual file. This is also what was done in
   the tutorial example above.

..
   TODO: asciinema of fixing broken transaction

..
   TODO: asciinema of brotli compression

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

..
   TODO: ascinema animation of asynchronous backup

Combining Local and Remote Concurrency
--------------------------------------

A client is made of two parts: the file, and the connection. A client can
handle concurrency for both parts simultaneously. That is to say, it is
possible for two different clients running on the same machine to share a
connection to the same remote server, and also share the same local file.

..
   TODO: ascinema animation of synchronous backup
