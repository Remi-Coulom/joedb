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

``Generic_File_Database``
-------------------------

``Generic_File_Database`` is a superclass of ``File_Database`` that takes a
reference to a ``Generic_File`` as parameter to its constructor, instead
of a file name. Subclasses of ``Generic_File`` allows accessing data in
various ways:

 - ``Stream_File`` uses a ``std::streambuf``.
 - ``Memory_File`` writes to a ``std::vector<char>`` in memory.
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

You can also create your own file class by subclassing ``Generic_File`` and
implementing its virtual functions.
