System Configuration
====================

Websocket Proxy with websockify and nginx
-----------------------------------------

.. code-block:: bash

 websockify --unix-target=./server.joedb.sock 8080

.. code-block:: nginx

 location /sockets/server.joedb.sock
 {
  proxy_pass http://localhost:8080/;
  proxy_http_version 1.1;
  proxy_set_header Upgrade $http_upgrade;
  proxy_set_header Connection "upgrade";
  proxy_read_timeout 5m;
  proxy_buffering off;
 }
