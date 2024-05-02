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
reference to a ``joedb::Generic_File`` as parameter to its constructor, instead
of a file name. Subclasses of ``joedb::Generic_File`` allows accessing data in
various ways:

 - ``joedb::Stream_File`` uses a ``std::streambuf``.
 - ``joedb::Memory_File`` writes to a ``std::vector<char>`` in memory.
 - ``joedb::Readonly_Memory_File`` reads from ``const char *``.
   :ref:`joedb_embed` can be used to embed a joedb database into a C++ string
   literal.
 - ``joedb::File`` is a typedef to either ``joedb::Windows_File``,
   ``joedb::Posix_File``, or ``joedb::Portable_File``. System-specific version
   of ``joedb::File`` offer extra features, such as locking, which is necessary
   to handle :doc:`concurrent <concurrency>` access to a file.
 - ``joedb::SFTP_File`` read-only access to a file via sftp (uses libssh).
 - ``joedb::CURL_File`` read-only access to a file via any URL (uses libcurl).
 - ``joedb::Interpreted_File`` can read joedbi commands directly.

You can also create your own file class by subclassing ``Generic_File`` and
implementing its pure virtual functions.

.. _file_slices:

File Slices (for Android APK)
-----------------------------

The Android NDK offers functions that return a file descriptor as well as a
position and size of an asset within the APK (see the NDK Android Asset
`Documentation <https://developer.android.com/ndk/reference/group/asset>`_).
It is possible to directly open such an asset without extracting it, using the
``set_slice`` member function of ``joedb::Generic_File``. Here is an example:

.. code-block:: c++

  joedb::Posix_File file(file_descriptor, joedb::Open_Mode::read_existing);
  file.set_slice(start, length);
  tutorial::Readonly_Database db(file);
