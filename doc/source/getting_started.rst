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
