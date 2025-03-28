API reference
=============

Files produced by the compiler
------------------------------

The compiler produces C++ source files, stored in a directory named after the
namespace of the database. For example, after compiling the joedb tutorial, you
can look at those files in the ``joedb/doc/source/tutorial/tutorial``
directory.

``readonly.h`` and ``readonly.cpp`` contain the code for reading databases.
``writable.h`` and ``writable.cpp`` contain the code for both reading and
writing databases. ``writable.cpp`` includes ``readonly.cpp`` so you do not
need to add both sources to your project.

``readonly.h`` and ``writable.h`` are provided for easy upgrade of code using
an old version of joedb. It is recommended to not include them in new code:
include individual headers for the classes you are actually using instead.

Joedb classes
-------------

`Doxygen <./doxygen/index.html>`_
