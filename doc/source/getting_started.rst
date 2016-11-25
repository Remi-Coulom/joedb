Getting Started
===============

This explains how to install and use joedb.

Compilation
-----------

The source code of joedb is available from `GitHub <https://github.com/Remi-Coulom/joedb>`_. Joedb is written in portable C++11, and uses `CMake <https://cmake.org/>`__ for its build system. It should be reasonably portable to many platforms, but because it was developed there, `Ubuntu <https://www.ubuntu.com/>`_ is likely to be the easiest one to use.

Linux, MacOS (and maybe other variations of Unix)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Prerequisites in Ubuntu can be installed with this command:

.. code-block:: bash

    sudo apt-get install g++ cmake ninja-build lcov unzip valgrind python-sphinx wget git-core sqlite3 libsqlite3-dev sqlitebrowser

Similar packages should be available in other systems. Joedb was compiled successfully on MacOS with `macports <https://www.macports.org>`_.

When the necessary packages are installed, the following commands should get you ready:

.. literalinclude:: ./tutorial/compiling.sh
   :language: bash

If you wish to install joedb system-wide, you can run:

.. code-block:: bash

    sudo ninja install

Windows
^^^^^^^

First, download and extract a zip file of a recent release from the `github Release page <https://github.com/Remi-Coulom/joedb/releases>`_.

`CMake <https://cmake.org>`__ is used to generate project files. In the CMake GUI, enter the ``compcmake`` directory in the field that says "Where is the source code:". Enter an empty or non-existing directory for "Where to build the binaries:", such as ``compcmake/build``, for instance. Then click "Configure", and "Generate". This should produce a ``joedb.sln`` for Visual Studio. CMake can also generate project files for other build environments.

Some binaries will fail to compile, because they require the joedb compiler to produce some C++ files first. But the main joedb tools will be compiled. Right now, there is not automatic way to generate the missing files. You should be able to produce them by running the joedb compiler manually.

Joedb Tools
-----------

Compiling joedb produces a few executables tools.

``joedbi``
^^^^^^^^^^
``joedbi`` is the joedb interpreter. When invoked without parameters, it will create a database in memory. When invoked with a parameter, it will open it as a file. A database will be created if it does not exist yet.

Below is a list of commands the interpreter understands:

.. literalinclude:: ./tutorial/joedbi_help.out
   :language: none

``joedbc``
^^^^^^^^^^

``joedbc`` is the joedb compiler. It takes two file names as parameter. The first file should contain joedbi commands to create the database schema. The second file contains compiler options.

The joedbc file should at least contain a ``namespace`` option that indicates the namespace in which the code will be generated. Other options indicate indexes, and which data structure should be used to store tables. These are explained in more details in the relevant sections: :doc:`indexes`, :doc:`vectors`.

``joedb_logdump``
^^^^^^^^^^^^^^^^^

``joedb_logdump`` takes a joedb file name as parameter, and produces a sequence of joedbi commands. With the ``--sql`` option, it can produce SQL output. This way, joedb data can be easily imported into any system that understands SQL.

For instance, this is the SQL output of the tutorial database:

.. literalinclude:: ./tutorial/logdump.sql
   :language: sql

``joedb_to_json``
^^^^^^^^^^^^^^^^^

``joedb_to_json`` takes a joedb file name as parameter, and produces json output. Each column is represented as a vector, and references are indexes into the vector (-1 indicates the null reference). The ``--base64`` option encodes strings in `base64 <https://en.wikipedia.org/wiki/Base64>`_. Joedb considers a string as a vector of bytes that may take any value, but json strings are limited to UTF-8. So ``--base64`` might be necessary for binary data or other special characters.

This is the json output of the tutorial database:

.. literalinclude:: ./tutorial/tutorial.json
   :language: json

``joedb_pack``
^^^^^^^^^^^^^^

``joedb_pack`` takes two file names as parameters. It will pack the first file into the second one. Packing a file removes all its history, and keeps only the most recent data.

In order to support schema recognition (see :doc:`schema_upgrade`), data definition commands are not packed. They are left as-is, at the beginning of the log, in the same order as in the original file.

``joedb_browser``
^^^^^^^^^^^^^^^^^

``joedb_browser`` uses ``joedb_logdump`` to produce an SQLite database, and invokes ``sqlitebrowser`` to browse it.

Example Code
------------

Example code is located in the ``doc/source/tutorial`` directory:

- ``tutorial.joedbi`` contains the interpreter commands that define the database schema,
- ``tutorial.joedbc`` defines compiler options,
- ``tutorial_main.cpp`` is the example presented in the :doc:`intro`,
- ``index_tutorial.cpp`` illustrates how to use :doc:`indexes`,
- and ``generate.sh`` is a bash script that will compile all the code and run the programs.

