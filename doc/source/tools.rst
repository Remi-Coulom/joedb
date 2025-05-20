.. _tools:

Tools
=====

Joedb comes with a collection of command-line tools.

``joedbi``
----------

.. literalinclude:: ./tutorial/joedbi.out
   :language: none

``joedbi`` is the joedb interpreter. It has commands to let you directly modify
a file, and visualize the content of tables.

Below is a list of commands the interpreter understands:

.. literalinclude:: ./tutorial/joedbi_help.out
   :language: none

.. _joedbc:

``joedbc``
----------

.. literalinclude:: ./tutorial/joedbc.out
   :language: none

``joedbc`` is the joedb compiler. It takes two file names as parameters. The
first file should contain joedbi commands to create the database schema. The
second file contains compiler options.

The joedbc file must at least contain a ``namespace`` option that indicates the
namespace in which the code will be generated. It can also be used to
:doc:`define indexes <indexes>`.

.. _joedb_logdump:

``joedb_logdump``
-----------------

.. literalinclude:: ./tutorial/joedb_logdump.out
   :language: none

``joedb_logdump`` takes a joedb file name as parameter, and produces a sequence of joedbi commands. With the ``--sql`` option, it can produce SQL output. This way, joedb data can be easily imported into any system that understands SQL.

For instance, this is the SQL output of the tutorial database:

.. literalinclude:: ./tutorial/logdump.sql
   :language: sql

.. _joedb_pack:

``joedb_pack``
--------------

.. literalinclude:: ./tutorial/joedb_pack.out
   :language: none

Packing a file removes all its history, and keeps only the most recent data.

In order to support schema recognition (see :doc:`schema_upgrade`),
data-definition commands are not packed. They are left as-is, at the beginning
of the log, in the same order as in the original file.


.. _joedb_convert:

``joedb_convert``
-----------------

.. literalinclude:: ./tutorial/joedb_convert.out
   :language: none

This copies the input to the output. ``--ignore-error`` sets the checkpoint
value to the file length, and can be used to recover a damaged file.

In case the joedb file format ever changes in a way that is not compatible with
the previous version, then this tool can be used to perform the conversion. The
new format is first implemented in write functions. At this moment,
``joedb_convert`` is still able to read the old format, and writes the new
format.

.. _joedb_merge:

``joedb_merge``
---------------

.. literalinclude:: ./tutorial/joedb_merge.out
   :language: none

``joedb_merge`` merges multiple files with the same schema into a single file that contains the concatenation of all tables. References are translated. Duplicates are not eliminated.

For instance, when merging those two databases:

.. literalinclude:: ./tutorial/merge_1.json
   :language: json

.. literalinclude:: ./tutorial/merge_2.json
   :language: json

``joedb_merge`` produces this result:

.. literalinclude:: ./tutorial/merged.json
   :language: json

.. _joedb_server:

``joedb_server``
----------------

.. literalinclude:: ./tutorial/joedb_server.out
   :language: none

Run a server to share a single database. See :doc:`concurrency` for more information.

.. _joedb_multi_server:

``joedb_multi_server``
----------------------

.. literalinclude:: ./tutorial/joedb_multi_server.out
   :language: none

.. _joedb_client:

``joedb_client``
----------------

.. literalinclude:: ./tutorial/joedb_client.out
   :language: none

.. _joedb_push:

``joedb_push``
----------------------

.. literalinclude:: ./tutorial/joedb_push.out
   :language: none

Pushes a local file to a connection. For example:

.. code-block:: bash

    # Dump content of a file
    joedb_push file.joedb --db dump

.. code-block:: bash

    # Truncate a file containing an aborted transaction
    joedb_push broken.joedb file fixed.joedb

.. code-block:: bash

    # Follow additions to the end of a file
    joedb_push --follow file.joedb --db dump

.. code-block:: bash

    # Keep a backup server updated
    joedb_push --follow database.joedb ssh user host ~/backup.joedb.sock

.. code-block:: bash

    # Export a joedb file to sqlite
    joedb_push database.joedb --db sql | sqlite3 database.sqlite

.. code-block:: bash

    # Compress a database
    joedb_push database.joedb file brotli compressed.joedb

.. _joedb_embed:

``joedb_embed``
---------------

.. literalinclude:: ./tutorial/joedb_embed.out
   :language: none

``joedb_embed`` compiles a joedb database file into a C++ string literal, and a function to open it as a ``Database``.

.. _joedb_to_json:

``joedb_to_json``
-----------------

``joedb_to_json`` takes a joedb file name as parameter, and produces json
output. Each column is represented as a vector, and references are indexes into
the vector (-1 indicates the null reference). The ``--base64`` option encodes
strings in `base64 <https://en.wikipedia.org/wiki/Base64>`_. Joedb considers a
string as a vector of bytes that may take any value, but json strings are
limited to UTF-8. So ``--base64`` might be necessary for binary data or other
special characters.

This is the json output of the tutorial database:

.. literalinclude:: ./tutorial/tutorial.json
   :language: json

.. _joedb_edit:

``joedb_edit``
-----------------

``joedb_edit`` is a bash script that dumps a joedb file to a temporary text
file and opens the default text editor to modify it. Modifications will be
pushed to the original if possible. Otherwise the user is given the option to
overwrite the original file.

``joedb_browser``
-----------------

``joedb_browser`` is a bash script that produces an SQLite database, and
invokes ``sqlitebrowser`` to browse it.
