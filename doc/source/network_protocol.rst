Network Protocol
================

checkpoint, size, and version information are sent as 64-bit small-endian numbers.

Client to Server
----------------

====== ================ ======================================================
Prefix Data             Description
====== ================ ======================================================
joedb  client_version   first message, sent at connection time
P      checkpoint       pull
L      checkpoint       lock-pull
U      checkpoint       push-unlock
       size
       data
l                       lock
u                       unlock
i                       ignore (used to keep the connection alive)
====== ================ ======================================================


Server to Client
----------------

====== ================ ======================================================
Prefix Data             Description
====== ================ ======================================================
joedb  server_version   reply to joedb.
                        server_version = 0 means client_version is rejected.
P      checkpoint       reply to P
       size
       data
L      checkpoint       reply to L
       size
       data
U                       reply to U when no timeout
l                       reply to l
u                       reply to u when no timeout
T                       reply to U or u, in case of timeout
i                       reply to i
====== ================ ======================================================
