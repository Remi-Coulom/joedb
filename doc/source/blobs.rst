.. _blobs:

Blobs
=====

Blobs are strings that are not automatically loaded into memory. It is a
convenient feature to store large pieces of data, such as photos or other
media. For each blob, only the position of the data in the joedb file is loaded
into memory. The content of a blob can be loaded on demand with the
``read_blob_data`` method of the file object.

Here is an example database schema:

.. literalinclude:: ../../test/compiler/db/blob.joedbi
   :language: joedbi

And the unit test that demonstrates how to use a blob:

.. literalinclude:: ../../test/compiler/Blob.cpp
   :language: c++

In order to get better performance when manipulating a large amount of blob
data, blobs can be stored in a separate file. This way, non-blob data will be
stored contiguously, and can be read much faster.

:joedb:`Server_File` allows accessing blobs from a remote server without having
to download a replica of the whole database locally. This is another good
reason to store tables separately from blobs if you have a lot of blobs.
