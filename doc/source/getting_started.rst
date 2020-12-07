Getting Started
===============

This explains how to install and use joedb.

Installing with a package
-------------------------

You can find some ready-made deb packages on the `github Release page <https://github.com/Remi-Coulom/joedb/releases>`_.

On Ubuntu, you can install joedb this way:

.. code-block:: bash

    sudo apt update
    sudo apt install ./joedb-4.0.0-Ubuntu-18.04-amd64.deb

Uninstall:

.. code-block:: bash

    sudo apt remove joedb

Compiling from source
---------------------

The source code of the most recent stable release can be found on the `github Release page <https://github.com/Remi-Coulom/joedb/releases>`_. If you are more adventurous you can also clone the repository to get the most recent version: ``git clone https://github.com/Remi-Coulom/joedb.git``.

Joedb is written in portable C++11, and uses `CMake <https://cmake.org/>`__ for
its build system. So it should be portable to almost any platform. Here are
some detailed instructions for the most common situations.

Linux
^^^^^

Minimal prerequisites in Ubuntu can be installed with this command:

.. code-block:: bash

    sudo apt install g++ cmake ninja-build libssh-dev

When the necessary packages are installed, the following commands should get you ready:

.. literalinclude:: ./tutorial/compiling.sh
   :language: bash

If you wish to install joedb system-wide, you can run:

.. code-block:: bash

    sudo ninja install
    sudo ldconfig

If you wish to run coverage tests, documentation, and benchmarks, also install:

.. code-block:: bash

    sudo apt-get install lcov python-sphinx python-sphinx-rtd-theme python-sphinxcontrib.spelling sqlite3 libsqlite3-dev sqlitebrowser

Windows
^^^^^^^

Visual Studio supports cmake, starting from version 2017. Use "Open a local folder" to open the ``compcmake`` folder of the archive.

If you wish to use the ssh connection, you can install the libssh library with
vcpkg: ``vcpkg install libssh:x64-windows``.

Joedb Tools
-----------

Compiling joedb produces a few executable tools.

``joedbi``
^^^^^^^^^^
``joedbi`` is the joedb interpreter. When invoked without parameters, it will create a database in memory. When invoked with a parameter, it will open it as a file. A database will be created if it does not exist yet.

Below is a list of commands the interpreter understands:

.. literalinclude:: ./tutorial/joedbi_help.out
   :language: none

.. _joedbc:

``joedbc``
^^^^^^^^^^

``joedbc`` is the joedb compiler. It takes two file names as parameters. The first file should contain joedbi commands to create the database schema. The second file contains compiler options.

The joedbc file should at least contain a ``namespace`` option that indicates the namespace in which the code will be generated. Other options indicate indexes, and which data structure should be used to store tables. These are explained in more details in the relevant sections: :doc:`indexes`, :doc:`vectors`.

.. _joedb_logdump:

``joedb_logdump``
^^^^^^^^^^^^^^^^^

.. literalinclude:: ./tutorial/joedb_logdump.out
   :language: none

``joedb_logdump`` takes a joedb file name as parameter, and produces a sequence of joedbi commands. With the ``--sql`` option, it can produce SQL output. This way, joedb data can be easily imported into any system that understands SQL.

For instance, this is the SQL output of the tutorial database:

.. literalinclude:: ./tutorial/logdump.sql
   :language: sql

.. _joedb_to_json:

``joedb_to_json``
^^^^^^^^^^^^^^^^^

``joedb_to_json`` takes a joedb file name as parameter, and produces json output. Each column is represented as a vector, and references are indexes into the vector (-1 indicates the null reference). The ``--base64`` option encodes strings in `base64 <https://en.wikipedia.org/wiki/Base64>`_. Joedb considers a string as a vector of bytes that may take any value, but json strings are limited to UTF-8. So ``--base64`` might be necessary for binary data or other special characters.

This is the json output of the tutorial database:

.. literalinclude:: ./tutorial/tutorial.json
   :language: json

.. _joedb_pack:

``joedb_pack``
^^^^^^^^^^^^^^

``joedb_pack`` takes two file names as parameters. It will pack the first file into the second one. Packing a file removes all its history, and keeps only the most recent data.

In order to support schema recognition (see :doc:`schema_upgrade`), data definition commands are not packed. They are left as-is, at the beginning of the log, in the same order as in the original file.

``joedb_browser``
^^^^^^^^^^^^^^^^^

``joedb_browser`` uses ``joedb_logdump`` to produce an SQLite database, and invokes ``sqlitebrowser`` to browse it.

.. _joedb_merge:

``joedb_merge``
^^^^^^^^^^^^^^^

``joedb_merge`` merges multiple files with the same schema into a single file that contains the concatenation of all tables. References are translated. Duplicates are not eliminated.

.. literalinclude:: ./tutorial/joedb_merge.out
   :language: none

For instance, when merging those two databases:

.. literalinclude:: ./tutorial/merge_1.json
   :language: json

.. literalinclude:: ./tutorial/merge_2.json
   :language: json

``joedb_merge`` produces this result:

.. literalinclude:: ./tutorial/merged.json
   :language: json

.. _joedb_embed:

``joedb_embed``
^^^^^^^^^^^^^^^

``joedb_embed`` compiles a joedb database file into a C++ string literal, and a function to open it as a ``Database``. ``embedded_test`` shows an example of use.

.. literalinclude:: ./tutorial/joedb_embed.out
   :language: none

.. _joedb_ssh_connect:

``joedb_ssh_connect``
^^^^^^^^^^^^^^^^^^^^^

``joedb_ssh_connect`` runs the ``joedbi`` interpreter concurrently on a shared
remote database.

.. literalinclude:: ./tutorial/joedb_ssh_connect.out
   :language: none

Example Code
------------

Example code is located in the ``doc/source/tutorial`` directory:

- ``tutorial.joedbi`` contains the interpreter commands that define the database schema,
- ``tutorial.joedbc`` defines compiler options,
- ``tutorial_main.cpp`` is the example presented in the :doc:`intro`,
- ``index_tutorial.cpp`` illustrates how to use :doc:`indexes`,
- and ``generate.sh`` is a bash script that will compile all the code and run the programs.

