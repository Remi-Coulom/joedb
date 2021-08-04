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
2. Flush data to disk (fsync).
3. Write the second copy of the checkpoint position.
4. Flush data to disk.

A checkpoint is considered valid if the two copies are identical.

The two checkpoints are used alternately. This way, if a crash occurs during a
checkpoint, it is still possible to recover the previous checkpoint.

Checkpoints and Transactions
----------------------------

A checkpoint does not necessarily indicate that data is in a valid and coherent
state. The purpose of the checkpoint is only to prevent data loss or corruption
in case of a crash. In particular, the error-management code will checkpoint
the log file before throwing an exception, which may occur in the middle of a
transaction. If needed, a separate ``valid_data`` event can be used to indicate
that data is valid.

Checkpoint Types
----------------

The joedb compiler produces three checkpoint functions:

- ``checkpoint_full_commit()``: Performs all 4 steps. Safe and slow.
- ``checkpoint_half_commit()``: Performs step 1, 2, and 3, but not 4. This is
  about twice faster than a full commit (if the commit is small). It ensures
  that data up to the previous checkpoint is safely recoverable. Data of the
  current checkpoint is written to disk, but recovery may require care if the
  second checkpoint copy does not make it to the disk before the crash.
- ``checkpoint_no_commit()``: Performs step 1 and 3 only. This does not flush
  data to permanent storage, but it flushes it to the operating system. This
  protects data from an application crash, but not from an operating-system
  crash. It is tremendously faster than full or half commit. It is still safe
  in the sense that, although the most recent modification may be lost, the
  database won't be corrupted, and all the data that had reached permanent
  storage before the system crash can be recovered safely.

The safety of the half_commit and no_commit versions depends on the operating
system, file system, and disk hardware. According to the SQLite documentation
[SQLite-AC]_, it seems that most modern file systems may exhibit the
safe-append property. In fact, the Redis database also uses an append-only
journal file without worrying about the risk of corruption [Redis-2016a]_.

Benchmarks
----------

The source code for these benchmarks can be found in the joedb/benchmark
directory. They were run on a Linux machine with an i7-5930K CPU, and WDC
WD20EZRX-00D8PB0 hard drive, with an ext4 file system.

Bulk Insert
~~~~~~~~~~~

The table below is the minimum of 10 runs, with N = 10,000,000 rows inserted.

+------+---------+--------+----------------+
|      | sqlite3 | joedb  | joedb (vector) |
+======+=========+========+================+
| real | 10.266s | 2.803s |         1.937s |
+------+---------+--------+----------------+
| user |  7.838s | 0.567s |         0.231s |
+------+---------+--------+----------------+
| sys  |  0.319s | 0.200s |         0.192s |
+------+---------+--------+----------------+

First the sqlite3 code:

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

  db.checkpoint_full_commit();

The joedb code not only uses 13 times less CPU time, it is also shorter, much more readable, and has many less potential run-time errors.

The performance of joedb can be further improved by using :doc:`vector insertions <vectors>`:

.. code-block:: c++

  {
   auto v = db.new_vector_of_benchmark(N);
   auto name = db.update_vector_of_name(v, N);
   auto value = db.update_vector_of_value(v, N);

   for (size_t i = 0; i < N; i++)
   {
    name[i] = "TOTO";
    value[i] = int64_t(i + 1);
   }
  }

  db.checkpoint_full_commit();

Commit Rate
~~~~~~~~~~~

Instead of one big commit at the end, each insert is now committed to disk one by one. With N = 100:

+------+---------+---------------------+---------------------+-------------------+
|      | sqlite3 | joedb (full_commit) | joedb (half_commit) | joedb (no_commit) |
+======+=========+=====================+=====================+===================+
| real | 5.434s  | 3.184s              | 1.549s              | 0.004s            |
+------+---------+---------------------+---------------------+-------------------+
| user | 0.006s  | 0.003s              | 0.002s              | 0.004s            |
+------+---------+---------------------+---------------------+-------------------+
| sys  | 0.021s  | 0.016s              | 0.009s              | 0.000s            |
+------+---------+---------------------+---------------------+-------------------+

Thanks to its simple append-only file structure, joedb can operate safely with
less synchronization operations than sqlite3, which makes it about 1.7 or 3.5
times faster, depending on synchronization mode.

Note also that joedb does not require a file system: it can also operate over a
raw device directly, which might offer additional opportunities for performance
optimization.
