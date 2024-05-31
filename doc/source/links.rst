Links
=====

Database software with similar ideas (with history)

- https://redis.io/ (the AOF persistence option is similar to joedb)
- https://kafka.apache.org/intro Kafka is an event streaming system
- https://samza.apache.org/
- https://prevayler.org/ (inactive)
- https://www.datomic.com/
- https://github.com/attic-labs/noms (inactive) a git-like database
- https://github.com/dolthub/dolt "git for data"

Structured data storage (with no history):

- https://developers.google.com/protocol-buffers/
- https://google.github.io/flatbuffers/
- http://sqlite.org/
- https://www.json.org/
- https://en.wikipedia.org/wiki/XML

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
- http://database-programmer.blogspot.fr/2008/07/history-tables.html

- https://www.reddit.com/r/programming/comments/3tfkdq/immutability_in_db_might_be_the_next_big_thing/
- https://www.youtube.com/watch?v=fU9hR3kiOK0
- `I Want Decentralized Version Control for Structured Data! <https://jonas-schuermann.name/projects/dvcs-for-structured-data/blog/2020-03-22-manifesto.html>`_

Crash resistance:

.. [Rosenblum-1991] `The Design and Implementation of a Log-Structured File System <https://people.eecs.berkeley.edu/~brewer/cs262/LFS.pdf>`_
.. [Redis-2016a] `Redis persistence demystified <http://oldblog.antirez.com/post/redis-persistence-demystified.html>`_
.. [Redis-2016b] `fsync() on a different thread: apparently a useless trick <http://oldblog.antirez.com/post/fsync-different-thread-useless.html>`_
.. [SQLite-AC] `Atomic Commit In SQLite <https://sqlite.org/atomiccommit.html>`_
.. [Moyer-2011] `Ensuring data reaches disk <https://lwn.net/Articles/457667/>`_
.. [serverfault-2009] `SATA Disks that handle write caching properly? <https://serverfault.com/questions/15404/sata-disks-that-handle-write-caching-properly>`_
.. [PG-2018] `PostgreSQL's fsync() surprise <https://lwn.net/Articles/752063/>`_

.. [PG-2019] `How is it possible that PostgreSQL used fsync incorrectly for 20 years, and what we'll do about it <https://archive.fosdem.org/2019/schedule/event/postgresql_fsync/>`_

Compression

- https://code.google.com/p/snappy/ (used by LevelDB)

Fast IO:

- https://github.com/ned14/llfio
- https://stackoverflow.com/questions/1201261/what-is-the-fastest-method-for-high-performance-sequential-file-i-o-in-c
- https://devblogs.microsoft.com/oldnewthing/20221130-00/?p=107505

File Locking:

- https://gavv.net/articles/file-locks/
- https://man7.org/linux/man-pages/man2/flock.2.html
- https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-lockfileex

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
