.. _blobs:

Blobs
=====

Blobs are strings that are not automatically loaded into memory. It is a
convenient feature to store large pieces of data, such as photos or other
media. For each blob, only the position of the data in the joedb file is loaded
into memory.  The content of a blob can be loaded on demand with the
``read_blob_data`` method of the file object.

Here is an example database schema:

.. literalinclude:: ../../test/compiler/db/blob.joedbi
   :language: joedbi

And the unit test that demonstrate how to use a blob:

.. literalinclude:: ../../test/compiler/Blob.cpp
   :language: c++

