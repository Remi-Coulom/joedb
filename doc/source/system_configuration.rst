System Configuration
====================

Websocket Proxy with websockify and nginx
-----------------------------------------

To circumvent firewalls that allow nothing but web traffic, the unix-domain
socket of a joedb_server can be served as a web socket.

https://github.com/novnc/websockify

.. code-block:: bash

 sudo apt install python3-websockify

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

https://github.com/tg123/websockify-nginx-module might be a better approach. But it seems it may have trouble with http2: https://github.com/tg123/websockify-nginx-module/issues/24
