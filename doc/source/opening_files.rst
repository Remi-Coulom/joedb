.. _opening_files:

Opening Files
=============

The simplest way to open a file is to pass a file name to the constructor of
either :joedb:`tutorial::File_Database` or
:joedb:`tutorial::Readonly_Database`.

.. code-block:: c++

  tutorial::File_Database db("file.joedb");

.. code-block:: c++

  tutorial::Readonly_Database db("file.joedb");

:joedb:`joedb::Open_Mode`
--------------------------

You can change the way the file is opened by passing an extra parameter to the
constructor of :joedb:`tutorial::File_Database`.  Available modes are:

.. literalinclude:: ../../src/joedb/journal/Open_Mode.h

``shared_write`` is dangerous, and requires synchronization. Users of the
library should not directly manipulate files with this mode, and instead use
:doc:`transactions <concurrency>`. Other write modes will use file locking to
prevent more than one process from writing to the same file simultaneously.
``write_lock`` is like ``write_existing_or_create_new``, but waits instead of
failing if anybody else is already write-locking.

For example:

.. literalinclude:: ./tutorial/src/file_tutorial.cpp
   :language: c++

:joedb:`tutorial::File_Client`
------------------------------

Writes to a :joedb:`tutorial::File_Database` must be manually checkpointed,
which is not very convenient. You can avoid having to explicitly checkpoint
your writes by using :ref:`transactions <file_client>` with a
:joedb:`tutorial::File_Client` instead.

:joedb:`Buffered_File`
----------------------

:joedb:`tutorial::File_Database` is a shortcut that allows opening a database
directly with a file name. Its more generic superclass
:joedb:`tutorial::Writable_Database` can access data stored in the various
specializations of the :joedb:`Buffered_File` class.
:joedb:`tutorial::Readonly_Database` can also take a read-only
:joedb:`Buffered_File` as constructor parameter.

Here are some specializations of :joedb:`Buffered_File`:

 - :joedb:`Stream_File` uses a ``std::streambuf``.
 - :joedb:`Memory_File` writes to a ``std::string``.
 - :joedb:`Readonly_Memory_File` reads from ``const char *``.
   :ref:`joedb_embed` can be used to embed a joedb database into a C++ string
   literal.
 - :joedb:`File_Slice` is a specialization of :joedb:`Readonly_Memory_File`
   that reads a range of bytes from a Posix file by memory-mapping it. It can
   be used to read Android assets (`Documentation
   <https://developer.android.com/ndk/reference/group/asset>`_).
 - :joedb:`File` is a typedef to either :joedb:`Windows_File`,
   :joedb:`Posix_File`, or :joedb:`Portable_File`. System-specific version of
   :joedb:`File` offer extra features, such as locking, which is necessary to
   handle :doc:`concurrent <concurrency>` access to a file.
 - :joedb:`SFTP_File` read-only access to a file via sftp (uses libssh).
 - :joedb:`CURL_File` read-only access to a file via any URL (uses libcurl).
 - :joedb:`Interpreted_File` can read joedbi commands directly.
 - :joedb:`Encoded_File` performs on-the-fly encoding and decoding of data.
   This can be used for transparent compression or encryption. Does not support
   concurrency and durability.
 - :joedb:`Upgradable_File` makes all write operations successful, but nothing
   is actually written to the file. This is convenient if you want to apply
   :doc:`automatic schema upgrades <schema_upgrade>` to a read-only file.
 - :joedb:`Server_File` allows accessing a file served by a :ref:`joedb_server`.

You can also create your own file class by subclassing :joedb:`Buffered_File`
and implementing the virtual functions of the :joedb:`Abstract_File` superclass.
