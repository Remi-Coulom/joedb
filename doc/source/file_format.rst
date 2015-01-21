File Format
===========

General
-------

5 bytes: joedb

version number uint32_t

table_id_t, field_id_t, record_id_t

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

Each checkpoint is a 64-bit file length, repeated twice. A checkpoint is valid if the two copies are identical. The current checkpoint is the highest valid checkpoint. Each checkpoint is written in alternance.

This checkpoint system is designed to ensure crash recovery. If the actual length of the file is not equal to the current checkpoint when opening a joedb file, then it is necessary to perform crash recovery.

Log Entry
---------

TODO: Better idea: should be documented in the code. Include pretty-printed C++
header file here.

+-----+--------------------+------------+-----------------------------------+
|Code | Meaning            |  Data      | Comment                           |
+=====+====================+============+===================================+
| 00  | End of file        |            |                                   |
+-----+--------------------+------------+-----------------------------------+
| 01  | Begin Transaction  |            |                                   |
+-----+--------------------+------------+-----------------------------------+
| 02  | End Transaction    |            |                                   |
+-----+--------------------+------------+-----------------------------------+
| 0e  | Time Stamp         | int64_t    | Milliseconds since the Epoch      |
+-----+--------------------+------------+-----------------------------------+
| 0f  | Tag                | string     |                                   |
+-----+--------------------+------------+-----------------------------------+
| 10  | Create Table       | string     | name of the new table             |
+-----+--------------------+------------+-----------------------------------+
| 11  | Add Field          | table_id_t | id of the table                   |
|     |                    +------------+-----------------------------------+
|     |                    | type_id_t  | type of the new field             |
|     |                    +------------+-----------------------------------+
|     |                    | string     | name of the new field             |
+-----+--------------------+------------+-----------------------------------+
