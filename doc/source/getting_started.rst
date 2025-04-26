Getting Started
===============

This explains how to install and use joedb.

Compiling from source
---------------------

The source code of joedb can be found on `joedb's github page
<https://github.com/Remi-Coulom/joedb/>`_. The master branch can be cloned with
``git clone https://github.com/Remi-Coulom/joedb.git``.

Joedb is written in portable C++ 17, and uses `CMake <https://cmake.org/>`_
for its build system, and `vcpkg <https://vcpkg.io/en/>`_ for its
dependencies. So it should be portable to almost any platform.

Note that joedb can work without any external dependency, as pure standard
C++. Each external dependency provides extra optional features.

Windows
^^^^^^^

Visual Studio can open the CMake project located in the ``compcmake`` folder.
``CMakePresets.json`` contains the ``vcpkg_release`` and ``vcpkg_debug``
presets that you can use to get all dependencies automatically with vcpkg.
You may have to install vcpkg in Visual Studio as explained on `that page
<https://devblogs.microsoft.com/cppblog/vcpkg-is-now-included-with-visual-studio/>`_.

Linux
^^^^^

Linux can use vcpkg, but cmake should be able to find installed system
packages as well. Full development prerequisites in Ubuntu can be installed
with this command:

.. code-block:: bash

    sudo apt install git g++ clang clang-tidy cmake make ninja-build libssh-dev libbrotli-dev libcurl4-openssl-dev libgtest-dev lcov python3-sphinx python3-sphinx-rtd-theme python3-sphinxcontrib.spelling sqlite3 libsqlite3-dev sqlitebrowser

When the necessary packages are installed, the following commands should
compile everything:

.. literalinclude:: ./tutorial/compiling.sh
   :language: bash

These commands will install joedb system-wide:

.. code-block:: bash

    sudo cmake --build . install
    sudo ldconfig

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

If you are using `cmake <https://cmake.org/>`_ to develop your own project
using joedb, you can handle dependencies automatically by including
joedb/compcmake/joedbc.cmake in your CMakeLists.txt. The tutorial source
contains an example:

.. literalinclude:: ./tutorial/CMakeLists.txt
   :language: cmake
