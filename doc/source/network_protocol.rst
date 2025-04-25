Network Protocol
================

all numbers are sent as 64-bit little-endian numbers

Client to Server
----------------

====== ================= ======================================================
Prefix Data              Description
====== ================= ======================================================
joedb  client_version    first message, sent at connection time
H      until             check SHA-256 hash code
       hash (32 bytes)
r      offset size       read a range of bytes

D      wait from         pull, no lock, no data
E      wait from         pull, lock, no data
F      wait from         pull, no lock, data
G      wait from         pull, lock, data

L                        lock
M                        unlock
N      from until data   push, keep locked
O      from until data   push, unlock
====== ================= ======================================================

Server to Client
----------------

====== ================ ======================================================
Prefix Data             Description
====== ================ ======================================================
joedb  | server_version | reply to joedb.
       | session_id     | server_version = 0 means client_version is rejected.
       | checkpoint     | 'R' is pull-only
       | 'R' or 'W'
H                       reply to H, hash is matching
h                       reply to H, hash mismatch
r      size data        reply to r (size may be shorter than what was sent)

D      until            reply to D
E      until            reply to E
F      until data       reply to F
G      until data       reply to G

L                       reply to L (success)
M                       reply to M (success)
N                       reply to N (success)
O                       reply to O (success)

R                       reply to E, G, L, M, N, O when the server is read-only
C                       reply to N, O in case of conflict
t                       reply to N, O in case of time out
====== ================ ======================================================
