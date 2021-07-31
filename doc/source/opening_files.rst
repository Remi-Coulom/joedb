.. _opening_files:

Opening Files
=============

The simplest way to open a file is to pass a file name to the constructor of
either ``File_Database`` or ``Readonly_Database``. But subclasses of the
``joedb::Generic_File`` class allows using various sources of data.

Directly passing file names to constructors
-------------------------------------------

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
   write_existing_or_create_new // mode of File_Database
  };

So ``write_existing`` and ``create_new`` are available only with this method.

Using a C++ Stream
------------------

``joedb::File`` is in fact a specialization of a more general ``joedb::Generic_File`` class that offers more flexibility. By subclassing ``joedb::Generic_File`` it is possible to let joedb use various ways to read and store data.

Two such subclasses are ``joedb::Stream_File`` and ``joedb::Input_Stream_File``, that take a ``std::iostream`` and ``std::istream`` as constructor parameter. The code below is an example of how to use them.

.. literalinclude:: ./tutorial/stream_tutorial.cpp
   :language: c++

Memory Files
------------

``joedb::Readonly_Memory_File`` and ``joedb::Memory_File`` allow reading or
writing joedb files from/to memory.

:ref:`joedb_embed` can be used to embed a joedb database into a C++ string
literal.

Opening Android Assets Directly from the APK
--------------------------------------------

The Android NDK offers functions that return a file descriptor as well as a position and size of an asset within the APK (see the NDK Android Asset `Documentation <https://developer.android.com/ndk/reference/group/asset>`_). It is possible to directly open such an asset without extracting it, using a ``File_Slice``. The constructor of a ``File_Slice`` takes 3 parameters: a C ``FILE*`` , a starting position, and a file length. It can be used as shown in the example below:

.. code-block:: c++

  FILE* file = fdopen(file_descriptor, "rb");
  joedb::File_Slice file_slice(file, start, length);
  tutorial::Readonly_Database db(file_slice);

Note: the destructor of joedb::File_Slice will fclose the file. You must not fclose it.

Class Hierarchy
---------------

.. code-block:: c++

  class Generic_File;

  // File is a typedef of one of these 3 system-specific classes:
  class Posix_File: public Generic_File;
  class Windows_File: public Generic_File;
  class Portable_File: public Generic_File;

  class Memory_File: public Generic_File;
  class Readonly_Memory_File: public Generic_File;

  class Stream_File: public Generic_File;

  class File_Slice: public Portable_File;
