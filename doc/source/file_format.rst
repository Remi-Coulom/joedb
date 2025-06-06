File Format
===========

Global File Structure
---------------------

===================== ========= ======================= ================
Range                 Name      Content                 Synchronization
===================== ========= ======================= ================
0...                  Head      version + checkpoint    Shared lock when reading the checkpoint.

                                                        Exclusive lock when writing the checkpoint,

                                                        and during journal construction.

41...                 Body      immutable journal       Never modified, never locked:

                                                        can be safely read without any synchronization.

checkpoint...         Tail      current transaction     Exclusive lock when writing the transaction,

                                                        and during journal construction.

===================== ========= ======================= ================

Head
----

Each joedb file starts with a 41-byte head that contains a file-format version
as well as checkpoints. Integer values are stored in little-endian format
(least significant byte first).

====== ======================= ==============================================
Offset Value                   Description
====== ======================= ==============================================
0      9e 00 00 00 00 00 00 00 uint64_t checkpoint 1, copy 1
8      9e 00 00 00 00 00 00 00 uint64_t checkpoint 1, copy 2
16     3d 01 00 00 00 00 00 00 uint64_t checkpoint 2, copy 1
24     3d 01 00 00 00 00 00 00 uint64_t checkpoint 2, copy 2
32     04 00 00 00             uint32_t file-format version
36     6a 6f 65 64 62          "joedb" file signature
41                             Beginning of the log
====== ======================= ==============================================

Each checkpoint is a 64-bit file length, repeated twice. A checkpoint is valid
if the two copies are identical. The current checkpoint is the highest valid
checkpoint. Each checkpoint is written alternately.

This checkpoint system is designed to ensure crash recovery. If the actual
length of the file is not equal to the current checkpoint when opening a joedb
file, then it is necessary to perform crash recovery.

More information can be found in the :doc:`checkpoints` section.

Compact Number Format
---------------------

Log entries use a compact number format for string length, record id, table id,
and field id. This format allows very large values, but still takes very little space for small values.

Bytes of compact numbers are stored with the most significant bytes first. The number of extra bytes is stored in the 3 high-order bits of the first byte. Numbers between 0 and 31 included are stored as one byte. At most 61 bits can be stored.

Here are some examples

====== ======================
Number Compact representation
====== ======================
0      00
1      01
2      02
...
31     1f
32     20 20
33     20 21
...
256    21 00
...
8191   3f ff
8192   40 20 00
...
====== ======================

Log Entry
---------

See ``joedb/journal/Readonly_Journal.cpp``.
