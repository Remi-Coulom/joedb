Getting Started
===============

This explains how to install and use joedb.

Compilation
-----------

The source code of joedb is available from `GitHub <https://github.com/Remi-Coulom/joedb>`_. Joedb is written in portable C++11, and uses `cmake <https://cmake.org/>`_ for its build system. It should be reasonably portable to many platforms, but because it was developed there, `Ubuntu <https://www.ubuntu.com/>`_ is likely to be the easiest one to use.

Linux, MacOS (and maybe other variations of Unix)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Prerequisites in Ubuntu can be installed with this command:

.. code-block:: bash

    sudo apt-get install g++ cmake ninja-build lcov unzip valgrind python-sphinx wget git-core sqlite3 libsqlite3-dev sqlitebrowser

Similar packages should be available in other systems. Joedb was compiled successfully on MacOS with `macports <https://www.macports.org>`_.

When the necessary packages are installed, the following commands should get you ready:

.. code-block:: bash

    git clone https://github.com/Remi-Coulom/joedb.git
    cd joedb/compcmake/
    ./get_gtest.sh
    ./generate.sh
    cd ninja_release
    ninja

If you wish to install joedb system-wide, you can run:

.. code-block:: bash

    sudo ninja install

Windows
^^^^^^^

If you use one of them, cygwin or Ubuntu on Windows might be the most convenient approach, and would look very much like the Linux method above.

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

For instance, this is the sql output of the tutorial database:

.. literalinclude:: ./tutorial/logdump.sql
   :language: sql

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
- ``tutorial_main.cpp`` is the cpp shown in the :doc:`intro`,
- ``index_tutorial.cpp`` illustrates how to use :doc:`indexes`,
- and ``generate.sh`` is a bash script that will compile all the code and run the programs.

