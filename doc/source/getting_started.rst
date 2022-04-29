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

Prerequisites in Ubuntu can be installed with this command (ninja and libssh
are optional, but nice to have):

.. code-block:: bash

    sudo apt install git g++ cmake ninja-build libssh-dev

For test coverage, documentation, and benchmarks, also install:

.. code-block:: bash

    sudo apt-get install lcov python3-sphinx python3-sphinx-rtd-theme python3-sphinxcontrib.spelling sqlite3 libsqlite3-dev sqlitebrowser

When the necessary packages are installed, the following commands should
compile everything:

.. literalinclude:: ./tutorial/compiling.sh
   :language: bash

These commands will install joedb system-wide:

.. code-block:: bash

    sudo cmake --build . install
    sudo ldconfig

Windows
^^^^^^^

First, get submodules:

.. code-block:: bash

    git submodule update --init --recursive

libssh can be installed with `vcpkg <https://github.com/microsoft/vcpkg>`_:

.. code-block:: bash

    vcpkg install libssh:x64-windows

Visual Studio can open the CMake project located in the ``compcmake`` folder.

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
CMakeLists.txt. The tutorial source contains an example

.. literalinclude:: ./tutorial/CMakeLists.txt
   :language: cmake
