Network Protocol
================

all numbers are sent as 64-bit little-endian numbers

Client to Server
----------------

====== ================= ======================================================
Prefix Data              Description
====== ================= ======================================================
joedb  client_version    first message, sent at connection time
r      offset            read a range of bytes
       size
P      checkpoint        pull
       wait_milliseconds
i      wait_milliseconds get server checkpoint (like P, without data)
H      checkpoint        check SHA-256 hash code
       hash (32 bytes)
Q                        quit
------ ----------------- ------------------------------------------------------
L      checkpoint        lock-pull
       wait_milliseconds
l      wait_milliseconds lock (like L, without data)
p      checkpoint        locked-push
       size
       data
U      checkpoint        push-unlock
       size
       data
u                        unlock
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
r      size             reply to r (size may be shorter than what was sent)
       data
P      checkpoint       reply to P
       size
       data
i      checkpoint       reply to i
H                       reply to H, hash is matching
h                       reply to H, hash mismatch
R                       reply to L, l, p, U, or u when the server is read-only
------ ---------------- ------------------------------------------------------
L      checkpoint       reply to L
       size
       data
l      checkpoint       reply to l
U                       reply to U or p when the push succeeded
C                       reply to U or p when the push failed (conflict)
u                       reply to u (no timeout)
t                       reply to u, U, or p in case of lock timeout
====== ================ ======================================================
