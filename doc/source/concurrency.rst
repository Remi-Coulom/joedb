.. _concurrency:

Concurrency
===========

Joedb offers mechanisms to access a single database from multiple processes.

Principle
---------

Concurrency works by letting each process have a local copy of the central
database. Each process can keep data synchronized with 4 basic operations:

- **lock**: get exclusive access to the central database. The lock operation may have to wait for another process to release its lock first.
- **unlock**: release the lock obtained by the previous operation.
- **pull**: update local data with new journal entries from the central database.
- **push**: transfer new local modification to the central database.

In order to keep coherency between multiple processes accessing the same central
database, the operations must follow some constraints:

- Editing the database must be done in this sequence: **lock**, then **pull**, then local edits, then **push**, and finally **unlock**.
- Synchronizing the data for reading is a sequence of **lock**, then **pull**, then **unlock**
- The local process is free to read the local data at any time, whether the central database is locked or not.

The C++ classes of joedb enforce these constraints automatically.

Example C++ code
----------------

SSH Connection
--------------

Note: for libssh to work in Windows, I had to convert my Linux private key this
way:

.. code-block:: none

    openssl rsa -in ./id_rsa -outform pem >id_rsa.pem

And rename the resulting id_rsa.pem to id_rsa.

Server
------

The most efficient way to handle concurrent access would be to use a server. A server would handle concurrency with a proper internal mutex instead of writing mutex files, which would be considerably more efficient. I'll implement one when the C++ networking TS becomes widely available.
