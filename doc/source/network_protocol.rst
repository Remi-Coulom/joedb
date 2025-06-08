Network Protocols
=================

The interaction between a client and a server is a simple iterative dialog
where the client sends a query, and waits for an answer from the server.

All values are sent as 64-bit little-endian numbers.

joedb_server
------------

Client to Server
~~~~~~~~~~~~~~~~

====== ================== ===================================================
Prefix Data               Description
====== ================== ===================================================
joedb  client_version     first message, sent at connection time
H      until              check SHA-256 hash code (fast)
       hash (32 bytes)
I      until              check SHA-256 hash code (full)
       hash (32 bytes)
r      from until         read a range of bytes
D      wait from          pull, no lock, no data
E      wait from          pull, lock, no data
F      wait from          pull, no lock, data
G      wait from          pull, lock, data
L                         lock
M                         unlock
N      from until data    push, keep locked
O      from until data    push, unlock
====== ================== ===================================================

Server to Client
~~~~~~~~~~~~~~~~

====== ================== ===================================================
Prefix Data               Description
====== ================== ===================================================
joedb  | server_version   | reply to joedb.
       | session_id       | server_version = 0: client_version is rejected.
       | checkpoint       | 'R' is pull-only
       | 'R' or 'W'
H                         reply to H, hash is matching
I                         reply to I, hash is matching
h                         reply to H or I, hash mismatch
r      until data         reply to r, size may be shorter than what was asked
D      until              reply to D
E      until              reply to E
F      until data         reply to F
G      until data         reply to G
L                         reply to L
M                         reply to M
N                         reply to N
O                         reply to O
R                         reply to E, G, L, M, N, O: the server is read-only
C                         reply to N, O in case of conflict
t                         reply to N, O in case of time out
====== ================== ===================================================

Remote Procedure Call
---------------------

Client to Server
~~~~~~~~~~~~~~~~

====== ================== ===================================================
Prefix Data               Description
====== ================== ===================================================
H      hash (32 bytes)    handshake, SHA-256 hash of procedures + version
P      id until data      remote procedure call
====== ================== ===================================================

Server to Client
~~~~~~~~~~~~~~~~

====== ================== ===================================================
Prefix Data               Description
====== ================== ===================================================
H      session_id         reply to H, hash is matching
h                         reply to h, hash mismatch
P      until data         reply to P (success)
p      size data          reply to P (error message)
====== ================== ===================================================
