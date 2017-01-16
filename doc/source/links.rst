Links
=====

Database with history/revisions:

- http://stackoverflow.com/questions/39281/database-design-for-revisions
- http://stackoverflow.com/questions/750782/database-design-for-text-revisions
- http://stackoverflow.com/questions/2724187/where-are-all-the-native-revisioned-databases
- http://stackoverflow.com/questions/9484714/how-to-store-datas-change-history
- http://stackoverflow.com/questions/9852703/store-all-data-changes-with-every-details-like-stackoverflow
- http://stackoverflow.com/questions/12321200/database-row-snapshots-revisions

- http://www.oracle.com/technetwork/issue-archive/2008/08-jul/o48totalrecall-092147.html
- http://www.zodb.org/en/latest/
- http://ayende.com/blog/162792/worlds-smallest-no-sql-database-persistent-transaction-logs
- http://database-programmer.blogspot.fr/2008/07/history-tables.html

- https://www.reddit.com/r/programming/comments/3tfkdq/immutability_in_db_might_be_the_next_big_thing/
- https://www.youtube.com/watch?v=fU9hR3kiOK0
- http://www.datomic.com/
- http://samza.apache.org/
- http://www.pgconf.us/2015/event/60/
- https://github.com/attic-labs/noms

Fast in-memory database with on-disk persistence

- http://redis.io/ (the AOF persistence option is very similar to joedb)

Crash resistance:

.. [Rosenblum-1991] `The Design and Implementation of a Log-Structured File System <http://www.cs.berkeley.edu/~brewer/cs262/LFS.pdf>`_
.. [Redis-2016a] `Redis persistence demystified <http://oldblog.antirez.com/post/redis-persistence-demystified.html>`_
.. [Redis-2016b] `fsync() on a different thread: apparently a useless trick <http://oldblog.antirez.com/post/fsync-different-thread-useless.html>`_
.. [SQLite-AC] `Atomic Commit In SQLite <http://sqlite.org/atomiccommit.html>`_
.. [Moyer-2011] `Ensuring data reaches disk <http://lwn.net/Articles/457667/>`_
.. [serverfault-2009] `SATA Disks that handle write caching properly? <http://serverfault.com/questions/15404/sata-disks-that-handle-write-caching-properly>`_

Compression

- https://code.google.com/p/snappy/ (used by LevelDB)

Fast IO:

- http://stackoverflow.com/questions/1201261/what-is-the-fastest-method-for-high-performance-sequential-file-i-o-in-c

SQLite:

- http://sqlite.org/

Berkeley DB:

- http://en.wikipedia.org/wiki/Berkeley_DB

Relational Template Library

- http://sourceforge.net/projects/rel-temp-lib/

POST++:

- http://www.garret.ru/post/readme.htm

On-disk containers:

- http://stackoverflow.com/questions/149488/disk-backed-stl-container-classes
- http://stxxl.sourceforge.net/

Object-Relational Mapping (ORM)

- http://www.codesynthesis.com/products/odb/
- http://www.webtoolkit.eu/wt/doc/tutorial/dbo/tutorial.html
- http://sourceforge.net/projects/litesql/
- https://github.com/rbock/sqlpp11

Active record pattern:

- http://en.wikipedia.org/wiki/Active_record_pattern

Serialization

- http://www.boost.org/doc/libs/1_56_0/libs/serialization/doc/index.html

C++

- queries: http://www.drdobbs.com/cpp/linq-like-list-manipulation-in-c/240166882
- db with templates: https://github.com/RamblingMadMan/dbpp

Interesting paper

- http://db.cs.yale.edu/vldb07hstore.pdf
