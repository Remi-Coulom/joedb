Checkpoints
===========

Understanding the Checkpoint Region of the Joedb File
-----------------------------------------------------

Checkpoint types
----------------

- ``checkpoint_no_commit()``
- ``checkpoint_half_commit()``
- ``checkpoint_full_commit()``

Benchmark of commit performance
-------------------------------

The source code for these benchmarks can be found in the joedb/benchmark directory. They were run on a Linux machine with an i7-5930K CPU, and WDC WD20EZRX-00D8PB0 hard drive.

Bulk Insert
~~~~~~~~~~~

The table below is the minimum of 10 runs, with N = 10,000,000 rows inserted.

+------+---------+--------+
|      | sqlite3 | joedb  |
+======+=========+========+
| real | 10.266s | 2.803s |
+------+---------+--------+
| user |  7.838s | 0.567s |
+------+---------+--------+
| sys  |  0.319s | 0.200s |
+------+---------+--------+

First the sqlite3 code:

.. code-block:: c++

  sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, 0);
  sqlite3_stmt *prepared_statement;
  sqlite3_prepare_v2(db,
                     "INSERT INTO BENCHMARK VALUES('TOTO', ?1)",
                     -1,
                     &prepared_statement,
                     0);

  for (int i = 1; i <= N; i++)
  {
   sqlite3_bind_int64(prepared_statement, 1, i);
   sqlite3_step(prepared_statement);
   sqlite3_reset(prepared_statement);
  }

  sqlite3_exec(db, "END TRANSACTION", 0, 0, 0);

Then, the equivalent joedb code:

.. code-block:: c++

  const std::string toto = "TOTO";

  for (int i = 1; i <= N; i++)
   db.new_benchmark(toto, i);

  db.checkpoint_full_commit();

The joedb code not only uses 13 times less CPU time, it is also shorter, much more readable, and has many less potential run-time errors.

Commit Rate
~~~~~~~~~~~

Instead of one big commit at the end, each insert is now committed to disk one by one. With N = 100:

+------+---------+--------------+--------------+
|      | sqlite3 | joedb (full) | joedb (half) |
+======+=========+==============+==============+
| real | 5.434s  | 3.184s       | 1.549s       |
+------+---------+--------------+--------------+
| user | 0.006s  | 0.003s       | 0.002s       |
+------+---------+--------------+--------------+
| sys  | 0.021s  | 0.016s       | 0.009s       |
+------+---------+--------------+--------------+

The half_commit version is reasonably crash-safe.

.. code-block:: c++

  for (int i = 1; i <= N; i++)
  {
   db.new_benchmark(toto, i);
   db.checkpoint_half_commit();
  }

The full_commit version is paranoid, completely safe, but twice slower:

.. code-block:: c++

  for (int i = 1; i <= N; i++)
  {
   db.new_benchmark(toto, i);
   db.checkpoint_full_commit();
  }

Thanks to its simple append-only file structure, joedb can operate safely with less synchronization operations than sqlite3, which makes it about 1.7 or 3.5 times faster, depending on synchronization mode.
