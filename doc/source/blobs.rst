.. _blobs:

Blobs
=====

Blobs are strings that are not automatically loaded into memory. It is a
convenient tool to store large pieces of data, such as photos. For each blob,
only the size and position of the data in the joedb file is loaded into memory.
The content of a blob can be loaded on demand with the ``read_blob`` method of
the file object.

Here is an example database schema:

.. literalinclude:: ../../test/compiler/db/blob.joedbi
   :language: joedbi

And the unit test that demonstrate how to use a blob:

.. literalinclude:: ../../test/compiler/Blob.cpp
   :language: c++
