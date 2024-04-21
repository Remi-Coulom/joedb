History
=======

- 2024-??-??: 8.0.0

  - joedb now uses C++17, and is not compatible with C++11 any more.
  - ``joedb_server`` now takes a client as parameter, instead of a file. This
    gives much more flexibility, and allows:

    - chaining servers for synchronous remote backup;
    - more generally, creating a tree of multiple database replicas;
    - local programs running on the same machine as the server can access the
      database file directly, without having to use a network connection to the
      server.

  - Some changes to better handle very large databases:

    - The server does not buffer a whole push before writing it to disk any
      more. This saves memory and time in case of a large push. If a connection
      fails in the middle of a push, the written data is not erased. But it is
      not checkpointed, and may be overwritten by a subsequent push.
    - Better timeouts: the timeout is not for the whole transaction, but for
      any network activity during the transaction. So, a very long transaction
      because of a very large push or pull will not time out if there is
      continuous network activity. Also, previous versions did not check for
      timeouts in the middle of a push. This is now fixed.

  - Performance optimizations:

    - ``File_Connection`` is now about 10 times faster.
    - Large string reads are also much faster.
    - Improved networking performance by fusing small consecutive socket
      writes. Previous version could be hit hard by Nagle's algorithm and
      delayed ACKs. This version uses ip::tcp::no_delay.

  - ``joedbc`` produces a new ``Pullable_Database``, similar to
    ``Readonly_Database``, but the file is not closed at the end of the
    constructor, and it is possible to pull new data in case of a concurrent
    update.
  - Classes that write a journal (``joedb::Writable_Journal``,
    ``Generic_File_Database``, ``File_Database``, ``Client``) now have two
    extra parameters:

    - ``check`` indicates the behaviour in case the file contains an incomplete
      transaction. It should be equal to either:

      - ``joedb::Readable_Journal::check::all`` (the default) fails if the file
        contains data after the checkpoint.
      - ``joedb::Readable_Journal::check::overwrite`` silently overwrite
        uncheckpointed data.

    - ``commit_level`` indicates the default commit level for checkpoints.
      Default is ``joedb::Commit_Level::no_commit``. See :doc:`Checkpoints
      <checkpoints>` for details.

  - ``id_of_x`` is now a literal type. All its member functions are
    ``constexpr``.
  - Minor fixes and improvements.

- 2023-08-15: 7.1.1

  - Bug fix of previous version: large pulls (>256kb) from a
    ``Readonly_File_Connection`` to an ``SFTP_File`` could fail.

- 2023-07-07: 7.1

  - New ``SFTP_File``, and ``Readonly_File_Connection``. Combining these
    classes allows read-only connection to a remote file without running a
    joedb server on the remote machine. This is convenient for periodic
    backups. SFTP does not support file locking in practice, so writing via
    SFTP is not implemented.

- 2023-05-25: 7.0

  - :ref:`Blobs <blobs>`
  - Deep reorganization of :doc:`concurrency <concurrency>`.

    - Ability to :ref:`safely share <local_and_remote_concurrency>` a single
      local replica of a remote server among multiple local clients.
    - Tools for :ref:`efficient remote asynchronous backup <backup_client>`.
    - Ability to :ref:`serve <joedb_server>` a file read-only.
    - New :ref:`joedb_push <joedb_push>` tool, with new interesting features
      such as the ability to follow a file or push to a remote backup server.

  - Removed some useless ssh code that was left over from the old serverless
    sftp connection. ``ssh::Thread_Safe_Sesion`` is renamed to
    ``ssh::Session``, and its constructor is faster than before because there
    is no sftp any more.
  - No more implicit conversion from compiled row id to integer or boolean.
    With this new version, explicit methods must be used: ``is_null()`` or
    ``is_not_null()`` to test if a reference is null or not, and ``get_id()``
    to convert to an integer. This ensures stronger typing, and prevents
    bug-prone implicit conversions.
  - Type-safe vector update of references. A range of ``id_of_x`` is passed
    instead of the generic ``Record_Id``.
  - No more endianness conversions: on big-endian machines joedb reads and
    writes data in big-endian format.
  - Minor fixes and improvements

- 2021-09-15: 6.0

  - new ``Local_Connection`` class for local serverless :doc:`concurrent
    <concurrency>` access to a file.
  - SHA-256 is used to compare the client database with the server database at
    connection time. This prevents pulling into the wrong file. This also
    allows making offline modifications to a local database, and pushing them
    later to a remote server.
  - Re-organize file classes:

    - ``File_Slice`` is removed. All file classes can be :ref:`sliced
      <file_slices>` now.
    - ``Stream_File`` uses a ``std::streambuf`` instead of ``std::iostream``.
    - new ``Interpreted_File`` can read joedbi commands directly.

  - Exception-safe :doc:`transactions <concurrency>`: if any exception is
    thrown by a client while writing, then none of what was written since the
    previous lock-pull will be pushed to the server. The previous approach,
    based on a Lock object, was defective and is not available any more.
  - New approach to :doc:`vector updates <vectors>` that allows testing for
    write errors (previous version wrote data in a destructor, which does not
    allow testing for errors).
  - Databases must be explictly checkpointed before destruction. The destructor
    won't checkpoint any more, because this would risk checkpointing a failed
    buffer flush, and because write errors cannot be handled properly in
    destructors.
  - Minor fixes and improvements.

- 2021-05-08: 5.0

  - Big improvements to :doc:`concurrency <concurrency>`: joedb now has a
    :doc:`network protocol <network_protocol>`, and a :ref:`server
    <joedb_server>` for efficient and reliable communication.
  - The old serverless ``SSH_Connection`` was removed completely. It was
    inefficient and unreliable. Using the server is much better.
  - Performance improvements of operations on large :doc:`vectors <vectors>`.
  - vim syntax highlighting for ``joedbi`` and ``joedbc`` files.
  - Minor fixes and improvements

- 2020-12-07: 4.0

  - :doc:`concurrency`: a new mechanism to allow multiple distributed processes
    to access the same remote database.
  - File sharing now works in Windows (one process can read a file while
    another is writing it, but two processes cannot open the same file for
    writing).
  - :ref:`joedb_embed` compiles the content of a database into a C++ string
    literal.
  - support for generating code inside a nested namespace (``namespace
    deeply::nested::tutorial`` in the .joedbc file).
  - .deb packages are provided for easy installation.
  - Minor fixes and improvements

- 2019-11-19: 3.0

  - More flexibility for opening files:

    - A database can be based on a C++ stream (which allows compression,
      encryption, or building a database into an executable as a string).
    - A read-only database can be opened directly from within an Android apk,
      without having to extract the file first.
    - See :ref:`opening_files` for more details.

  - Better portability:

    - Defining the ``JOEDB_PORTABLE`` macro builds joedb with portable C++ only
      (no file locking, no fsync). With this option, joedb can be used on the
      PlayStation 4 and the Nintendo Switch.
    - Unlike in Linux, ``fseek`` and ``ftell`` are 32-bit in Windows. So the
      previous version could not handle files larger than 2^31 bytes. This is
      now fixed, and very large files can be used in Windows.
    - Unlike Linux, Windows does no print any information when a program is
      terminated by an exception. Joedb tools in this version catch all
      exceptions, and print them before quitting.

  - Main version number incremented because of one minor change: custom
    functions are now member of ``Generic_File_Database`` instead of the
    ``File_Database`` class.

  - Minor fixes and improvements.

- 2018-04-02: 2.1

  - new :ref:`joedb_merge` tool to concatenate joedb files
  - dense table storage is more memory-efficient in the interpreter
  - Minor fixes and improvements

- 2017-01-18: 2.0

  - Exceptions everywhere: no more error codes, no more bad states, better
    diagnostics.
  - Safety: several safety checks were added. This version was thoroughly
    fuzzed, and should not crash on any input file. Many assertions were added
    to detect data-manipulation errors (double delete, double insert, reading
    invalid rows, etc.).
  - Better handling of read-only files and locking. A file opened for writing
    can now be opened for reading by other processes. Readers won't be updated
    by changes made by the writer, but it is still more convenient than before.
  - The compiler can produce a rudimentary C wrapper around the C++ classes.
  - :ref:`joedb_to_json`
  - Tested on big-endian and 32-bit machines
  - Many minor fixes and improvements

- 2016-11-18: 1.0
