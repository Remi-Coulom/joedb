Checkpoints
===========

In order to allow safe recovery from a crash, joedb uses a checkpoint technique
inspired by the Sprite Log-Structured File System [Rosenblum-1991]_. A
checkpoint indicates a position in the log up to which all the events have been
properly written.

Two copies of two checkpoint positions are stored in the beginning of the
:doc:`joedb file <file_format>`. A checkpoint is written in 4 steps:

1. Write all log entries up to this checkpoint, and the first copy of the
   checkpoint position.
2. file.sync() (flush data and metadata (file size) to storage)
3. Write the second copy of the checkpoint position.
4. file.datasync() (flush data to storage)

A checkpoint is considered valid if the two copies are identical.

The two checkpoints are used alternately. This way, if a crash occurs during a
checkpoint, it is still possible to recover the previous checkpoint.

On most modern file systems, the size of the file can be used as the second
copy of the checkpoint position, since it will be written to storage in a
second step, after writing data. But writing two checkpoint copies in the
joedb file itself makes the format independent from the file system. It even
makes it possible to write a joedb database to a raw device directly.

Soft Checkpoints
----------------

The checkpointing method described above is durable, but very slow. Joedb
offers an alternative "soft" checkpoint that does not call fsync. Soft
checkpoints are stored in the joedb header as negative values, to
differentiate them from hard checkpoints. Soft checkpoints never overwrite the
value of the hard checkpoint, so it will always be possible to safely recover
from the most recent hard checkpoint in case of power failure.

By default, all joedb tools use soft checkpoints. A hard checkpoint can be
written, by either executing it manually, or setting an option for
:joedb:`Client` and :joedb:`Server`.

.. _crash:

Recovering from a Crash
-----------------------

If a crash occurs in the middle of a transaction, then the file may end up
containing a dirty uncheckpointed tail. In such a situation, in order to
prevent any risk of data loss, joedb will refuse to open the file for writing.

If an error is detected, ``joedb_logdump --header`` provides detailed
information about the size of the file, and the values of the soft and hard
checkpoints. Recovering the journal until the hard checkpoint should be
completely safe. Recovering until the soft checkpoint is very likely to be safe
if it is shorter than the file size. It is also possible to recover until the
very end of the file, but that is much more risky. Journal truncation can be
performed with the :ref:`joedb_push` tool.

If you do not wish to manually recover from a crash, you can also tell joedb to
automatically recover from the most recent valid checkpoint, and silently
overwrite the uncheckpointed tail with :joedb:`Construction_Flags`::overwrite.

Checkpoints and Concurrency
---------------------------

In addition to durability, another important purpose of checkpoints is to
notify other processes of changes to a joedb file. Joedb uses an exclusive lock
when writing checkpoints, and a shared lock when reading them (see
:doc:`concurrency`, and :doc:`file_format` for more details). This allows
proper synchronization of simultaneous access to the same database.

File System Support
-------------------

Joedb relies on locks and fsync to handle concurrency and durability, but
those features are not available for all file systems. This is particularly
true when mounting a remote drive. Beware that sshfs, does not support file
locking or fsync at all. Depending on its version and how it is configured,
NFS may or may not properly support file locking and fsync. Samba, on the
other hand, seems to handle concurrency rather well.

WSL is another source of major file-locking pitfalls: locks applied from within
WSL and from the Windows host ignore each other
(`<https://github.com/microsoft/WSL/issues/5762>`_). It seems that the 9P file
server used by WSL does not support file locking. A workaround for this problem
consists in `using samba
<https://documentation.ubuntu.com/server/how-to/samba/mount-cifs-shares-permanently/index.html>`_
to mount Windows drives in WSL (or, conversely, launch a samba server in WSL2,
and mount the drive in Windows).

Even when properly configured for concurrency and durability, the performance
of file locking over NFS can be very bad in case of contention: on Ubuntu 24.04
with nfsv4, it takes 30 seconds for a client to find out that a lock it is
waiting for has been released. It seems that the NFS protocol does not provide
a way for the server to notify clients when a lock has been released, so the
client has to poll the server from time to time, which makes good performance
impossible. So, although it may sometimes work correctly, concurrency over NFS
is very inefficient.

For distributed access to a database, :ref:`joedb_server` provides considerably
better reliability and performance.

Benchmarks
----------

The source code for these benchmarks can be found in the joedb/benchmark
directory. They were run on an Ubuntu 24.04 machine with an AMD Ryzen 7 5800X
CPU, and a 2Tb Corsair MP600 NVMe SSD, with an encrypted ext4 file system.

Bulk Insert
~~~~~~~~~~~

The table below is the minimum of 10 runs, with N = 100,000,000 rows inserted.

+------+---------+--------+----------------+
|      | sqlite3 | joedb  | joedb (vector) |
+======+=========+========+================+
| real | 28.600s | 6.532s |         2.963s |
+------+---------+--------+----------------+
| user | 27.562s | 3.725s |         1.433s |
+------+---------+--------+----------------+
| sys  |  0.895s | 2.758s |         1.348s |
+------+---------+--------+----------------+

First the sqlite3 code (without error checking):

.. code-block:: c++

  sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, 0);
  sqlite3_stmt* prepared_statement;
  sqlite3_prepare_v2
  (
   db,
   "INSERT INTO BENCHMARK VALUES('TOTO', ?1)",
   -1,
   &prepared_statement,
   0
  );

  for (int i = 1; i <= N; i++)
  {
   sqlite3_bind_int64(prepared_statement, 1, i);
   sqlite3_step(prepared_statement);
   sqlite3_reset(prepared_statement);
  }

  sqlite3_exec(db, "END TRANSACTION", 0, 0, 0);

Then, the equivalent joedb code:

.. code-block:: c++

  for (int i = 1; i <= N; i++)
   db.new_benchmark("TOTO", i);

  db.hard_checkpoint();

The joedb code is not only faster, it is also shorter, much more readable,
and has many less potential run-time errors.

The performance of joedb can be further improved by using :doc:`vector insertions <vectors>`:

.. code-block:: c++

  {
   auto v = db.new_vector_of_benchmark(N);

   db.update_vector_of_name(v, N, [N](joedb::Span<std::string> name)
   {
    for (size_t i = 0; i < N; i++)
     name[i] = "TOTO";
   });

   db.update_vector_of_value(v, N, [N](joedb::Span<int64_t> value)
   {
    for (size_t i = 0; i < N; i++)
     value[i] = int64_t(i + 1);
   });
  }

  db.hard_checkpoint();

Writing large vectors is faster than inserting elements one by one in a loop,
especially for primitive types.

Commit Rate
~~~~~~~~~~~

Instead of one big commit at the end, each insert is now committed to disk one
by one. With N = 10,000:

+------+---------+----------+
|      | sqlite3 |   joedb  |
+======+=========+==========+
| real | 24.937s | 19.101s  |
+------+---------+----------+
| user |  0.175s |  0.028s  |
+------+---------+----------+
| sys  |  1.523s |  0.641s  |
+------+---------+----------+

There is much less difference in performance compared to a big transaction, but
joedb is still faster.

Joedb's soft checkpoint is similar in terms of durability to SQLite's WAL mode
with synchronous=NORMAL: after a power failure, some of the most recently
written data may be lost, but it is possible to recover safely to a recent
consistent state. With N = 1,000,000:

+------+---------+----------+
|      | sqlite3 |   joedb  |
+======+=========+==========+
| real | 12.826s |  2.639s  |
+------+---------+----------+
| user |  2.751s |  0.320s  |
+------+---------+----------+
| sys  |  5.945s |  2.316s  |
+------+---------+----------+

In addition to performance, one great advantage of joedb's soft checkpoints is
that, unlike SQLite's WAL, it works over a network filesystem.
