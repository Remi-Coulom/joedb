File Format
===========

General
-------

5 bytes: joedb

version number uint32_t

32 bytes: checkpoint region

log_entry*

little-endian storage

Checkpoint Region
-----------------

======== ===========================
Data     Comment
======== ===========================
uint64_t file length (checkpoint #1)
uint64_t file length (checkpoint #1)
uint64_t file length (checkpoint #2)
uint64_t file length (checkpoint #2)
======== ===========================

Each checkpoint is a 64-bit file length, repeated twice. A checkpoint is valid if the two copies are identical. The current checkpoint is the highest valid checkpoint. Each checkpoint is written alternately.

This checkpoint system is designed to ensure crash recovery. If the actual length of the file is not equal to the current checkpoint when opening a joedb file, then it is necessary to perform crash recovery.

Compact number format
---------------------

Big-endian order. Store the number of extra bytes in the 3 high-order bits of the first byte. Numbers between 0 and 31 included are stored as one byte. At most 61 bits can be stored. Used for string length, record id, table id, field id.

Log Entry
---------

Should be documented in the code. Include pretty-printed C++ header file here.
