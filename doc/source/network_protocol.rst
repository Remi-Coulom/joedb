Network Protocol
================

checkpoint, wait_milliseconds, size, version, and session_id are sent as 64-bit
little-endian numbers

Client to Server
----------------

====== ================= ======================================================
Prefix Data              Description
====== ================= ======================================================
joedb  client_version    first message, sent at connection time
P      checkpoint        pull
       wait_milliseconds
L      checkpoint        lock-pull
       wait_milliseconds
p      checkpoint        locked-push
       size
       data
U      checkpoint        push-unlock
       size
       data
u                        unlock
H      checkpoint        check SHA-256 hash code
       hash (32 bytes)
i                        ping (used to keep the connection alive)
Q                        quit
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
P      checkpoint       reply to P
       size
       data
L      checkpoint       reply to L
       size
       data
U                       reply to U or p when the push succeeded
C                       reply to U or p when the push failed (conflict)
R                       reply to L, p, U or u when the server is read-only
u                       reply to u (no timeout)
t                       reply to u, U, or p in case of timeout
H                       reply to H, hash is matching
h                       reply to H, hash mismatch
i                       reply to i
====== ================ ======================================================
