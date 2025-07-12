.. _rpc:

Remote Procedure Call
=====================

Joedb can be used as a binary message serialization format for remote procedure
calls.

Motivation
----------

:doc:`Journal replication <concurrency>` can be used to implement concurrency
over a network connection, but it has drawbacks:

 - All clients have to download the whole database. It is OK for a small number
   of clients or a small database, but it is a problem for a large number of
   concurrently writing clients: the cost of all clients downloading
   each-other's writes is quadratic in the number of clients, which does not
   scale.
 - When connecting over an unreliable network connection, a write transaction
   may leave the server locked. The server timeout can get rid of stale locks
   after a while, but it will halt all processing for the duration of the
   timeout. This is not acceptable when the server has to remain available.

So journal sharing is a great way to handle backups, caching, or local
concurrency, but is not efficient when many remote clients are writing
independent parts of the database over an unreliable network connection.

These scalability and reliability issues can be solved by using remote
procedure calls. With this mechanism, a disconnection can never leave a stale
lock, and there is no need for clients to download the whole database. Each
client can upload new data to the server in a very minimal single round-trip.
The write transaction will be executed entirely on the server, and cannot be
interrupted by a disconnection, so lock-timeout stalls cannot occur.

Tutorial Example
----------------

This example implements 3 procedures for the joedb tutorial database:

 - ``insert_city`` takes a string parameter and inserts it as a city name in
   the city table.
 - ``delete_city`` takes a string parameter and deletes the city with this
   name.
 - ``get_population`` takes a list of cities as parameter, and returns the
   number of persons for each city.

First a ``Service`` class has to be implemented to run the procedures on the
server:

.. literalinclude:: ./tutorial/src/tutorial.rpc/Service.h
   :language: c++
   :caption:

Then a channel can be used to connect a client to the server:

.. literalinclude:: /tutorial/src/rpc_client.cpp
   :language: c++
   :caption:

Here are the schema definitions for the procedure messages:

.. literalinclude:: ./tutorial/src/tutorial.rpc/city.joedbi
   :language: joedbi
   :caption:

.. literalinclude:: ./tutorial/src/tutorial.rpc/city.joedbc
   :language: joedbc
   :caption:

.. literalinclude:: ./tutorial/src/tutorial.rpc/population.joedbi
   :language: joedbi
   :caption:

.. literalinclude:: ./tutorial/src/tutorial.rpc/population.joedbc
   :language: joedbc
   :caption:
