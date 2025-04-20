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
made to a local database without any synchronization with the remote server.
It will still be possible to push those changes to the server when connecting
later, but the push will succeed only if there is no conflict.

Example
-------

The compiler produces code that ensures that locks and unlocks are correctly
paired, and modifications to the database can only occur during a lock.  This
is done with transaction function that takes a lambda as parameter, and
executes it between a lock_pull and a push_unlock.

.. literalinclude:: ./tutorial/src/concurrency_tutorial.cpp
   :language: c++

It produces this output:

.. literalinclude:: ./tutorial/concurrency_tutorial.out
   :language: none

Connections
-----------

The constructor of the :joedb:`tutorial::Client` class takes two parameters: a
file for storing the database journal, and a connection. The connection is an
object of the :joedb:`Connection` class, that provides the synchronization
operations (pull, lock_pull, push_unlock). This section presents the different
kinds of available connections.

Plain :joedb:`Connection`
^^^^^^^^^^^^^^^^^^^^^^^^^

The :joedb:`Connection` superclass does not connect to anything. Such a
connection can be used in a client to handle concurrent access to a local file.
If the file was opened with :joedb:`Open_Mode`::shared_write, clients can start
write transactions simultaneously, and they will be synchronized with file
locking.

.. _file_client:

:ref:`joedbc <joedbc>` produces a convenient :joedb:`tutorial::File_Client`
class that creates the connection and the client in a single line of code. Here
is an example:

.. literalinclude:: ./tutorial/src/local_concurrency.cpp
   :language: c++

Multiple instances of this program can safely write to the same database
concurrently.

.. asciinema:: ./asciinema/local_concurrency.cast
   :poster: npt:0:12

:joedb:`File_Connection`
^^^^^^^^^^^^^^^^^^^^^^^^

:joedb:`File_Connection` creates a connection to a file:

 - :joedb:`File_Connection` can be used to make a safe and clean copy of a database
   that contains a transaction that was not checkpointed, either because the
   database is currently being written to, or because of a previous crash.
 - :joedb:`File_Connection` can be used to convert between different file
   formats.  For instance, pushing a plain joedb file to a :joedb:`Brotli_File`
   will create a compressed database.
 - A :joedb:`File_Connection` to an :joedb:`SFTP_File` can be a convenient way
   to pull from a remote database without running a joedb server on the remote
   machine.  Performance will be inferior to running a joedb server, though.
   Similarly, a :joedb:`File_Connection` to a :joedb:`CURL_File` can be used to
   pull from a joedb database served by a web server.

..
   TODO: asciinema of fixing broken transaction

..
   TODO: asciinema of brotli compression

:joedb:`Server_Connection`
^^^^^^^^^^^^^^^^^^^^^^^^^^

:joedb:`Server_Connection` allows connecting to a running :ref:`joedb_server`
using the joedb :doc:`network protocol <network_protocol>`.

The constructor of :joedb:`Server_Connection` takes a :joedb:`Channel`
parameter. Two channel classes are provided:

 * :joedb:`Network_Channel` opens a network socket to the server directly.
 * :joedb:`ssh::Forward_Channel` connects to the server with ssh encryption and authentication.

.. _local_and_remote_concurrency:

Combining Local and Remote Concurrency
--------------------------------------

A client can handle concurrency for both its file and its connection
simultaneously: it is possible for two different clients running on the same
machine to share a connection to the same remote server, and also share the
same local file. For this to work, the local file must be opened with
:joedb:`Open_Mode`::shared_write.

Using a :joedb:`Client_Lock` instead of a Lambda
------------------------------------------------

The transaction function is a simple way to handle the
lock-pull-write-push-unlock sequence, but may not be flexible enough to handle
some more complex use cases. The :joedb:`Client_Lock` object allows:

 - starting the transaction in one function, and finishing it in another one,
 - pushing multiple times in the middle of a transaction, without unlocking the connection,
 - writing data in one thread, and asynchronously pushing from time to time
   in another one (use a mutex).

:joedb:`Client_Lock` performs :joedb:`Connection::lock_pull` in its
constructor, and you have to explicitly call either
:joedb:`Client_Lock::push_unlock` or :joedb:`Client_Lock::unlock` right before
its destruction.

.. literalinclude:: ./tutorial/src/client_lock.cpp
   :language: c++

..
   TODO: ascinema animation of synchronous backup

..
   TODO: ascinema animation of asynchronous backup
