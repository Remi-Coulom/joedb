.. _rpc:

Remote Procedure Call
=====================

:doc:`Journal replication <concurrency>` can be used to
implement concurrency over a network connection, but it has drawbacks:

 - All clients have to download the whole database, which does not scale.
 - The server may have to wait for a disconnected client to time out, which
   hurts availability.

These scalability and availability issues can be solved by using remote
procedure calls instead. With this mechanism, a disconnection can never leave a
stale lock, and there is no need for clients to download the whole database.
Each client can upload new data to the server in a very minimal single
round-trip. A write transaction will be executed entirely on the server, so its
duration is not affected by network latency, and it cannot be stalled by a
disconnection.

Joedb can be used as a binary message serialization format for remote procedure
calls. From a list of functions that take a writable database as parameter, the
joedb compiler can generate code for a client and a server that allow executing
these functions remotely.

Tutorial Example
----------------

This example implements 4 procedures for the joedb tutorial database:

 - ``insert_city`` takes a string parameter and inserts it as a city name in
   the city table.
 - ``delete_city`` takes a string parameter and deletes the city from the city
   table.
 - ``get_population`` takes a list of cities as parameter, and returns the
   number of persons for each city.
 - ``get_inhabitants`` takes a city name as parameter, and returns the list of
   persons living in this city

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
