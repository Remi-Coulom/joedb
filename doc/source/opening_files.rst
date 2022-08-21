.. _opening_files:

Opening Files
=============

The simplest way to open a file is to pass a file name to the constructor of
either ``File_Database`` or ``Readonly_Database``. But subclasses of the
``joedb::Generic_File`` class allows using various sources of data.

Database Constructors
---------------------

Opening a file for reading and writing (creates the file if it does not exist):

.. code-block:: c++

  tutorial::File_Database db("file.joedb");

Opening a file for reading only (throws an exception if it does not exist):

.. code-block:: c++

  tutorial::Readonly_Database db("file.joedb");

Using a ``joedb::File``
-----------------------

Opening the database with a ``joedb::File`` gives better control of the way the file is opened.

This is an example:

.. literalinclude:: ./tutorial/file_tutorial.cpp
   :language: c++

Available modes are:

.. code-block:: c++

  enum class Open_Mode
  {
   read_existing, // mode of Readonly_Database
   write_existing,
   create_new,
   write_existing_or_create_new, // mode of File_Database
   shared_write
  };

So ``write_existing``, ``create_new``, and ``shared_write`` are available only
with this method.

``shared_write`` is dangerous, and requires synchronization. Users of the
library should not directly open files with this mode, and instead use
:doc:`transactions <concurrency>` if they wish to write to a file that may be
opened simultaneously by multiple processes. Other write modes will use file
locking to prevent more than one process from writing to the same file
simultaneously.

Using a ``joedb::Generic_File``
-------------------------------

``joedb::File`` is in fact a specialization of a more general
``joedb::Generic_File`` class that offers more flexibility. By subclassing
``joedb::Generic_File`` it is possible to let joedb use various ways to read
and store data.

Available subclasses:

 - ``joedb::Stream_File`` uses a ``std::streambuf``.
 - ``joedb::Memory_File`` writes to a ``std::vector<char>`` in memory.
 - ``joedb::Readonly_Memory_File`` reads from ``const char *``.
   :ref:`joedb_embed` can be used to embed a joedb database into a C++ string
   literal.
 - ``joedb::File`` is a typedef to either ``joedb::Windows_File``, ``joedb::Posix_File``, or ``joedb::Portable_File``.
 - ``joedb::Interpreted_File`` can read joedbi commands directly.

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
