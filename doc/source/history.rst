History
=======

- 2025-??-?? 10.1.0

  - New major features:

    - :doc:`rpc`
    - :joedb:`joedb::streambuf`
    - websocket support

  - Minor improvements:

    - :joedb:`Robust_Connection` does not try to reconnect for every exception.
      Errors such as conflicts, or writing to a read-only server will not cause
      a retry any more, which prevents pointless infinite retry loops.
    - joedb_client option to set the keep-alive interval when connecting to a
      server. Warning: there is no keep-alive by default any more.
    - joedbc generates a .gitignore
    - joedbc does not generate irrelevant functions for single-row tables
      (sorting, vector updates)
    - joedbc generates functions to print tables (see :joedb:`print_city_table`)
    - :joedb:`Memory_File` can be opened with :joedb:`Open_Mode`::shared_write.
    - :joedb:`Connection::dummy`, used as default constructor parameter for clients.
    - writable :joedb:`File_View`, now built from a :joedb:`Buffered_File`, and
      overriding all :joedb:`Abstract_File` functions.

  - Fixes (also available on 10.0.0 LTS branch):

    - Fix joedb_push to a file connection
    - Fix MacOS installation (set RPATH)
    - Add a timeout to ssh session (could get infinite waiting without it)
    - Fix joedbc crash when no namespace is provided
    - Silence irrelevant "Ahead of checkpoint" warning when writing to an :joedb:`Interpreted_File`.

- 2025-05-26 10.0.0 LTS

  This a long-term support release: it will be updated with bug fixes, without
  breaking compatibility.

  - New features:

    - new ``delete_vector`` operation
    - Transactions can return a value.
    - :joedb:`Robust_Connection` automatically reconnects after an error.
    - :ref:`Server_File` allows accessing a remote database without
      downloading a local replica.
    - :joedb:`Client::pull` can wait for another client to make a
      push if the pull would otherwise be empty.
    - :joedb:`Encoded_File` supports on-the-fly coding or decoding of data.
      :joedb:`Brotli_Codec` is provided for compression.
    - It is possible to write to an :joedb:`Interpreted_File`.
    - The ``add_field`` interpreter command accepts an optional ``= <value>``
      suffix that sets the value for all existing records of the table.
    - If the .joedbi file provided to the compiler contains data, then it will
      be used as default initial value for existing records when creating a new
      field during automatic schema upgrade.
    - ``set_single_row <table> true`` in .joedbc file forces a table to contain
      a single row: the row is inserted automatically right after table
      creation, and the new and delete operations are not available. Instead,
      the ``the_<table>()`` member function returns a reference to the unique
      row of this table.
    - :joedb:`Upgradable_File` allows applying automatic schema upgrades to a
      read-only file.
    - :ref:`joedb_edit` allows editing a binary joedb file with a text editor.

  - Fixes and improvements:

    - Fix hash calculation of large files. ``joedb_server`` could wrongly
      report hash mismatch errors when there was no error.
    - Fixed some potential resource leaks when throwing from constructors in
      :joedb:`Posix_File`, :joedb:`ssh::Session`, :joedb:`ssh::SFTP`,
      :joedb:`ssh::Forward_Channel`, :joedb:`CURL_File`,
      :joedb:`Windows_File`.
    - Properly lock the tail of a shared file during journal construction. Not
      doing so could wrongly trigger an error when checking for file size.
    - The construction of a compiled client first reads the local file
      completely before running the schema-upgrade transaction. This improves
      concurrency by making the transaction much shorter if the local file is
      big.

  - Incompatibilities with previous version:

    - Record ids start at 0 instead of 1. The null reference is -1.
    - ``joedb_server`` uses unix domain sockets instead of tcp/ip sockets
    - File format changed: ``joedb_convert`` from branch ``convert_4_to_5``
      can upgrade old file to the new format.
    - Checkpoints changed: use either ``soft_checkpoint`` or
      ``hard_checkpoint``. See :doc:`checkpoints` for details
    - ``Generic_File`` was renamed to :joedb:`Buffered_File`
    - ``Generic_File_Database`` was renamed to :joedb:`Writable_Database`
    - ``Local_Client`` was renamed to :joedb:`File_Client`
    - ``read_blob_data`` was renamed to ``read_blob``
    - ``write_blob_data`` was renamed to ``write_blob``
    - The order of parameters of the Client's constructor are swapped: the file
      is first, then the connection.
    - The ``is_end_of_file()`` function was removed. Trying to read past the
      end of a file now throws an exception.
    - boolean values are printed as ``false`` and ``true`` instead of 0 and 1.
    - hashing functions were moved into a separate ``File_Hasher`` class.
    - ``Generic_File::set_mode`` and ``get_mode`` were removed. They are
      replaced by the more restrictive ``make_readonly()``, ``is_shared()``,
      and ``is_readonly()``;
    - ``generate_c_wrapper`` compiler option was removed.
    - ``set_table_null_initialization`` compiler option was removed.
    - code generated by the compiler is organized differently. See
      :doc:`api_reference` for details.

- 2024-06-25 9.0.1

  - Dual locking: instead of using one global lock for a joedb file, this
    version locks head and tail separately. This allows a much nicer handling
    of concurrent access to files:

    - Journal construction locks the head only, so it does not block if a
      transaction is in progress or the file was opened in exclusive mode since
      both of those situations lock the tail only.
    - Concurrent reads use a shared lock on the head of the file to read the
      checkpoint, and can be blocked only during very short periods of time
      when a writable journal is constructed or when the checkpoint is
      modified.

  - Write access to exclusive and shared files is completely unified, so
    ``Connection`` and ``File_Connection`` can now handle both exclusive and
    shared files.
  - Explicit handling of pull-only connections. The server can now cleanly
    serve a read-only file.
  - New ``CURL_File`` allows opening any URL as a read-only file. An http
    server supporting range access can serve a read-only database.
  - SQL dump connection
  - Generated code produces an error if compiled with a version of joedb
    different from the version that was used to generate it.
  - Incompatibilities with previous version:

    - In Posix environments, locking changed from using ``flock`` to using
      ``fcntl``. Those two locking mechanisms are not compatible in Linux, so
      it is important to avoid mixing joedb versions because they may not
      understand each-other's locks. Windows and MacOS do not have this
      problem.
    - The network protocol changed to indicate a pull-only connection during
      handshake, so it is not compatible with the previous version.
    - ``Local_Connection`` is removed since it can be replaced by a plain
      ``Connection``.
    - ``Pullable_Database`` is removed and replaced by ``Readonly_Client``.

- 2024-04-23: 8.0.1

  - Fix missing test for fsync error.

- 2024-04-21: 8.0.0

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
      Default is ``joedb::Commit_Level::no_commit``. See :doc:`checkpoints` for
      details.

  - ``id_of_x`` is now a literal type. All its member functions are
    ``constexpr``.
  - Minor fixes and improvements.

- 2023-08-15: 7.1.1

  - Bug fix of previous version: large pulls (>256kB) from a
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
    - Tools for efficient remote asynchronous backup.
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

    - ``File_Slice`` is removed. All file classes can be sliced now.
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
