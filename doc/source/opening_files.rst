.. _opening_files:

Opening Files
=============

The simplest way to open a file is to pass a file name to the constructor of
either ``File_Database`` or ``Readonly_Database``.

.. code-block:: c++

  tutorial::File_Database db("file.joedb");

.. code-block:: c++

  tutorial::Readonly_Database db("file.joedb");

``joedb::Open_Mode``
--------------------

You can change the way the file is opened by passing an extra parameter to the
constructor of ``File_Database``.  Available modes are:

.. code-block:: c++

  enum class Open_Mode
  {
   read_existing, // mode of Readonly_Database
   write_existing,
   create_new,
   write_existing_or_create_new, // default mode of File_Database
   shared_write,
   write_lock
  };

``shared_write`` is dangerous, and requires synchronization. Users of the
library should not directly manipulate files with this mode, and instead use
:doc:`transactions <concurrency>`. Other write modes will use file locking to
prevent more than one process from writing to the same file simultaneously.
``write_lock`` is like ``write_existing_or_create_new``, but waits instead of
failing if anybody else is already write-locking.

For example:

.. literalinclude:: ./tutorial/file_tutorial.cpp
   :language: c++

``File_Client``
---------------
Writes to a ``File_Database`` must be manually checkpointed
(``db.checkpoint()``), which is not very convenient. You can avoid having to
explicitly checkpoint your writes by using :ref:`transactions <file_client>`
with a ``File_Client`` instead.

``Buffered_File``
-----------------

``File_Database`` is a shortcut that allows opening a database directly with a
file name. Its more generic superclass ``Writable_Database`` can access data
stored in the various specializations of the ``Buffered_File`` class.
``Readonly_Database`` can also take a read-only ``Buffered_File`` as
constructor parameter.

Here are some specializations of ``Buffered_File`` provided by joedb:

 - ``Stream_File`` uses a ``std::streambuf``.
 - ``Memory_File`` writes to a ``std::string``.
 - ``Readonly_Memory_File`` reads from ``const char *``.
   :ref:`joedb_embed` can be used to embed a joedb database into a C++ string
   literal.
 - ``File_Slice`` is a specialization of ``Readonly_Memory_File`` that reads a
   range of bytes from a Posix file by memory-mapping it. It can be used to
   read Android assets (`Documentation
   <https://developer.android.com/ndk/reference/group/asset>`_).
 - ``File`` is a typedef to either ``Windows_File``,
   ``Posix_File``, or ``Portable_File``. System-specific version
   of ``File`` offer extra features, such as locking, which is necessary
   to handle :doc:`concurrent <concurrency>` access to a file.
 - ``SFTP_File`` read-only access to a file via sftp (uses libssh).
 - ``CURL_File`` read-only access to a file via any URL (uses libcurl).
 - ``Interpreted_File`` can read joedbi commands directly.
 - ``Encoded_File`` performs on-the-fly encoding and decoding of data. This can
   be used for transparent compression or encryption. Does not support
   concurrency and durability.
 - ``Upgradable_File<File_Type>`` makes all write operations successful, but
   nothing is actually written to the file. This is convenient if you want to
   apply :doc:`automatic schema upgrades <schema_upgrade>` to a read-only file.

You can also create your own file class by subclassing ``Buffered_File`` and
implementing its virtual functions.
