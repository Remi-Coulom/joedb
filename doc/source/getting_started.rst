Getting Started
===============

This explains how to install and use joedb.

Installing with a package
-------------------------

You can find some ready-made deb packages on the `github Release page <https://github.com/Remi-Coulom/joedb/releases>`_.

On Ubuntu, joedb can be installed this way:

.. code-block:: bash

    sudo apt update
    sudo apt install ./joedb-5.0.0-Ubuntu-18.04-amd64.deb

Uninstall:

.. code-block:: bash

    sudo apt remove joedb

Compiling from source
---------------------

The source code of the most recent stable release can be found on the `github
Release page <https://github.com/Remi-Coulom/joedb/releases>`_. You can also
clone the repository to get the most recent development version: ``git clone
https://github.com/Remi-Coulom/joedb.git``.

Joedb is written in portable C++11, and uses `CMake <https://cmake.org/>`_ for
its build system. So it should be portable to almost any platform. Here are
some detailed instructions for the most common situations.

Linux
^^^^^

Prerequisites in Ubuntu can be installed with this command (libssh and libboost
are not necessary if you don't wish to use the server):

.. code-block:: bash

    sudo apt install g++ cmake ninja-build libssh-dev libboost-dev

When the necessary packages are installed, the following commands should
compile everything:

.. literalinclude:: ./tutorial/compiling.sh
   :language: bash

These commands will install joedb system-wide:

.. code-block:: bash

    sudo ninja install
    sudo ldconfig

To run coverage tests, documentation, and benchmarks, also install:

.. code-block:: bash

    sudo apt-get install lcov python3-sphinx python3-sphinx-rtd-theme python3-sphinxcontrib.spelling sqlite3 libsqlite3-dev sqlitebrowser wget unzip doxygen graphviz python3-breathe

Windows
^^^^^^^

Visual Studio can open the CMake project located in the ``compcmake`` folder.
For the network connections, dependencies can be installed with `vcpkg
<https://github.com/microsoft/vcpkg>`_:

.. code-block:: bash

    vcpkg install libssh:x64-windows boost-asio:x64-windows

The ``install`` target of this project will produce a directory in
``joedb/compcmake/out/install``. You may have to copy the generated files
elsewhere, or adjust your system's PATH in order to make the tools easily
available on the command line.

First Steps
-----------

After downloading joedb, you might wish to look at examples located in the
``doc/source/tutorial`` directory:

- ``tutorial.joedbi`` contains the interpreter commands that define the database schema,
- ``tutorial.joedbc`` defines compiler options,
- ``tutorial_main.cpp`` is the example presented in the :doc:`intro`,
- ``index_tutorial.cpp`` illustrates how to use :doc:`indexes`,
- and ``generate.sh`` is a bash script that will compile all the code and run the programs.

It might be a good idea to also look at the :doc:`tools` provided by joedb, and
also read the rest of this User's Guide: it presents the most significant
features of joedb in more details than the Introduction.

Using joedb with cmake
----------------------

If you are using cmake to develop your own project using joedb, you can handle
dependencies automatically by including joedb/compcmake/joedbc.cmake in your
CMakeLists.txt. This is what it contains:

.. literalinclude:: ../../compcmake/joedbc.cmake
   :language: cmake

You may have to adjust the HINTS so that it finds the location of your joedbc.

This cmake script relies on the convention that the file name for your joedbi and joedbc files are the same as the namespace. So, if you have ``my_db.joedbi`` and ``my_db.joedbc`` in the ``my_directory`` directory, you can simply add the following line to tell cmake how to use joedbc to generate the C++ source files:

.. code-block:: cmake

    joedb_build(my_directory my_db)

Note that using cmake with source files generated by custom commands can be a
little tricky. You can find a lengthy explanation in a nice `blog post by Sam
Thursfield
<https://samthursfield.wordpress.com/2015/11/21/cmake-dependencies-between-targets-and-files-and-custom-commands/>`_.
If you don't wish to figure out the details, you can simply add the following
line, for each of your target that depends on source files generated by joedbc:

.. code-block:: cmake

    add_dependencies(my_target compile_mydb_with_joedbc)
