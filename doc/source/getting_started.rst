Getting Started
===============

This explains how to install and use joedb.

Compilation
-----------

The source code of joedb is available from `GitHub <https://github.com/Remi-Coulom/joedb>`_.

In Linux, the following commands should get you ready:

.. code-block:: bash

    sudo apt-get install libsqlite3-dev cmake ninja-build g++ lcov unzip valgrind python-sphinx wget git-core libboost-all-dev sqlitebrowser
    git clone https://github.com/Remi-Coulom/joedb.git
    cd joedb/compcmake/
    ./get_gtest.sh
    ./generate.sh
    cd ninja_release
    ninja

If you wish to install joedb system-wide, you can run:

.. code-block:: bash

    sudo ninja install

This will produce ``joedbi``, the joedb interpreter, and ``joedbc``, the joedb compiler. ``joedbi`` lets you manipulate the database with interactive commands. ``joedbc`` reads a file with joedbi commands that define the database schema, and produce C++ code as output.

Tests
-----

If you wish to test that everything is allright, you can run
``./quick_test.sh`` in the test directory, and ``./compiler_test.sh`` in the
test/compiler directory.

Tutorial
--------

All the files of the tutorial are located in the ``doc/source/tutorial`` directory. This directory contains 4 files: ``tutorial.joedbi`` contains the interpreter commands that define the database schema, ``tutorial.joedbc`` defines compiler options, ``tutorial_main.cpp`` is the cpp file that manipulates the database, and ``generate.sh`` is a bash script that will compile all the code and run the program.

Joedb Tools
-----------

Joedb comes with a few executable files.

``joedbi``
^^^^^^^^^^

``joedbi`` is the joedb interpreter. When invoked without parameters, it will create a database in memory. When invoked with a parameter, it will open it as a file. A database will be created if it does not exist yet.

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
