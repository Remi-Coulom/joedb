.. _opening_files:

Opening Files
=============

Joedb offers different ways to open a file. The simplest way is to pass a file
name to the constructor of either ``File_Database`` or ``Readonly_Database``.
But subclasses of the ``joedb::Generic_File`` class allows using various
sources of data, when passed as parameter to the constructor of either
``Generic_File_Database`` or ``Generic_Readonly_Database``.

With a file name
----------------

The simplest way to open a database is to give a file name directly.

Opening a file for reading and writing (creates the file if it does not exist):

.. code-block:: c++

  tutorial::File_Database db("file.joedb");

Opening a file for reading only (throws an exception if it does not exist):

.. code-block:: c++

  tutorial::Readonly_Database db("file.joedb");

Using a joedb::File
-------------------

The method described in the previous section is a shortcut.

Using a C++ stream
------------------

``joedb::File`` is in fact a specialization of a more general ``joedb::Generic_File`` class that offers more flexibility. By subclassing ``joedb::Generic_File`` it is possible to let joedb use various ways to read and store data.

.. literalinclude:: ./tutorial/stream_tutorial.cpp
   :language: c++

For the stream to work for writing, it must be seekable (no ``std::ios::app`` open mode). Files should be opened with ``std::ios::binary``.

Opening Android assets directly from the apk
--------------------------------------------

The Android NDK offers functions that return a file descriptor as well as a
position and size of an asset within the apk (see the NDK Android Asset
`Documentation <https://developer.android.com/ndk/reference/group/asset>`_). It
is possible to directly open such an asset without extracting it, using a
``File_Slice``. The constructor of a ``File_Slice`` takes 3 parameters: a C
``FILE*`` , a starting position, and a file length. It can be used as shown in
the example below:

.. code-block:: c++

  FILE* file = fdopen(file_descriptor, "rb");
  joedb::File_Slice file_slice(file, start, length);
  tutorial::Generic_Readonly_Database db(file_slice);

Class Hierarchy
---------------

.. code-block:: c++

  // The Database class manages in-memory table storage.
  // It has methods to read table content, but not to write it.
  class Database;

  // These are read-only databases based on a joedb::File
  class Generic_Readonly_Database: public Database;
  class Readonly_Database: public Generic_Readonly_Database;

  // These are writeable databases based on a joedb::File
  class Generic_File_Database: public Database;
  class File_Database: public Generic_File_Database;
