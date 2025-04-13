Links
=====

Database software with history

- https://redis.io/ (the AOF persistence option has similarities to joedb)
- https://kafka.apache.org/intro Kafka is an event streaming system
- https://samza.apache.org/
- https://prevayler.org/ (inactive)
- https://www.datomic.com/
- https://github.com/attic-labs/noms (inactive) a git-like database
- https://github.com/dolthub/dolt "git for data"

Streaming replication, continuous archiving, point-in-time recovery

- https://www.postgresql.org/docs/current/warm-standby.html
- https://www.postgresql.org/docs/current/continuous-archiving.html
- https://github.com/benbjohnson/litestream

Structured data storage (no history, no durable incremental update):

- https://developers.google.com/protocol-buffers/
- https://google.github.io/flatbuffers/
- https://www.json.org/
- https://en.wikipedia.org/wiki/XML
- https://www.boost.org/doc/libs/1_85_0/libs/serialization/doc/index.html
- http://www.garret.ru/post/readme.htm

On-disk containers:

- http://stackoverflow.com/questions/149488/disk-backed-stl-container-classes
- http://stxxl.sourceforge.net/

Using SQL from C++

- https://sqlite.org/ (non-durable transactions: https://www.sqlite.org/wal.html#fast)
- https://github.com/rbock/sqlpp11
- https://www.webtoolkit.eu/wt/doc/tutorial/dbo.html
- https://codesynthesis.com/products/odb/
- https://sourceforge.net/projects/litesql/
- http://en.wikipedia.org/wiki/Active_record_pattern
- C++20 ❤ SQL - John Bandela - CppCon 2021

  - https://www.youtube.com/watch?v=-P9FyevnL6U
  - https://cppcon.digital-medium.co.uk/wp-content/uploads/2021/10/Cpp20SQL.pdf
  - https://github.com/google/cpp-from-the-sky-down/tree/master/meta_struct_20/cppcon_version

Compression

- https://github.com/google/snappy (used by LevelDB)
- https://github.com/google/brotli
- https://facebook.github.io/zstd/
- https://lz4.org/
- https://www.percona.com/blog/evaluating-database-compression-methods/

Fast IO:

- https://github.com/ned14/llfio
- https://stackoverflow.com/questions/1201261/what-is-the-fastest-method-for-high-performance-sequential-file-i-o-in-c
- https://devblogs.microsoft.com/oldnewthing/20221130-00/?p=107505

File Locking:

- https://gavv.net/articles/file-locks/
- https://man7.org/linux/man-pages/man2/flock.2.html
- https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-lockfileex

Other links:

- https://stackoverflow.com/questions/39281/database-design-for-revisions
- https://stackoverflow.com/questions/750782/database-design-for-text-revisions
- https://stackoverflow.com/questions/2724187/where-are-all-the-native-revisioned-databases
- https://stackoverflow.com/questions/9484714/how-to-store-datas-change-history
- https://stackoverflow.com/questions/9852703/store-all-data-changes-with-every-details-like-stackoverflow
- https://stackoverflow.com/questions/12321200/database-row-snapshots-revisions
- https://stackoverflow.com/questions/68323888/how-to-implement-a-real-time-data-logging-backup-system
- https://opendata.stackexchange.com/questions/748/is-there-a-git-for-data

- https://blogs.oracle.com/connect/post/managing-history
- https://zodb.org/en/latest/
- https://ayende.com/blog/162792/worlds-smallest-no-sql-database-persistent-transaction-logs
- https://database-programmer.blogspot.fr/2008/07/history-tables.html

- https://www.reddit.com/r/databasedevelopment/
- https://www.reddit.com/r/programming/comments/3tfkdq/immutability_in_db_might_be_the_next_big_thing/
- https://www.youtube.com/watch?v=fU9hR3kiOK0
- `I Want Decentralized Version Control for Structured Data! <https://jonas-schuermann.name/projects/dvcs-for-structured-data/blog/2020-03-22-manifesto.html>`_
- `The SQLite team is preparing an efficient remote replication tool <https://www.reddit.com/r/programming/comments/1fvp9dz/the_sqlite_team_is_preparing_an_efficient_remote/>`_

Object retirement:
- `Andrezj's C++ blob: Operation cancelling and std::fstream <https://akrzemi1.wordpress.com/2019/05/23/operation-cancelling-and-stdfstream/>`
- `C++’s best feature <https://akrzemi1.wordpress.com/2013/07/18/cs-best-feature/>`
- `Throwing Destructors <https://www.kolpackov.net/projects/c++/eh/dtor-1.xhtml>`
- https://stackoverflow.com/questions/15112219/guarantee-a-certain-function-is-called-before-destruction

Crash resistance:

.. [Rosenblum-1991] `The Design and Implementation of a Log-Structured File System <https://people.eecs.berkeley.edu/~brewer/cs262/LFS.pdf>`_
.. [Redis-2016a] `Redis persistence demystified <http://oldblog.antirez.com/post/redis-persistence-demystified.html>`_
.. [Redis-2016b] `fsync() on a different thread: apparently a useless trick <http://oldblog.antirez.com/post/fsync-different-thread-useless.html>`_
.. [SQLite-AC] `Atomic Commit In SQLite <https://sqlite.org/atomiccommit.html>`_
.. [Moyer-2011] `Ensuring data reaches disk <https://lwn.net/Articles/457667/>`_
.. [serverfault-2009] `SATA Disks that handle write caching properly? <https://serverfault.com/questions/15404/sata-disks-that-handle-write-caching-properly>`_
.. [PG-2018] `PostgreSQL's fsync() surprise <https://lwn.net/Articles/752063/>`_
.. [PG-2019] `How is it possible that PostgreSQL used fsync incorrectly for 20 years, and what we'll do about it <https://archive.fosdem.org/2019/schedule/event/postgresql_fsync/>`_
.. [Miller-2024] `Userland Disk I/O <https://transactional.blog/how-to-learn/disk-io>`_

Papers

- `The End of an Architectural Era (It’s Time for a Complete Rewrite) <https://dslam.cs.umd.edu/vldb07hstore.pdf>`_
- `Are You Sure You Want to Use MMAP in Your Database Management System? <https://db.cs.cmu.edu/papers/2022/cidr2022-p13-crotty.pdf>`_, `YouTube presentation of the paper <https://www.youtube.com/watch?v=1BRGU_AS25c>`_

Data Loss Stories

- https://about.gitlab.com/blog/2017/02/10/postmortem-of-database-outage-of-january-31/
- https://www.reddit.com/r/ExperiencedDevs/comments/1j2wrdv/wiped_my_companys_production_db_last_week/
- https://www.brentozar.com/archive/2015/02/9-ways-to-lose-your-data/
