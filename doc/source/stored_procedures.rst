.. _stored_procedures:

Stored Procedures
=================

:doc:`Journal replication <concurrency>` can be used to implement concurrency
over a network connection, but it has drawbacks:

 - All clients have to download the whole database. It is OK for a small number
   of clients or a small database, but it is a problem for a large number of
   concurrently writing clients: the cost of all clients downloading
   each-other's writes is quadratic in the number of clients, which does not
   scale.
 - When connecting over an unreliable network connection, a write transaction
   may leave the server locked. This can be handled by setting the server's
   timeout, but it will be a source of bad performance, and trying to handle
   timeouts gracefully is difficult.

So journal sharing is a great way to handle backups, caching, or local
concurrency, but is not efficient when a large number of remote clients are
writing independent parts of the database over an unreliable network
connection.

Joedb offers an alternative to journal replication by allowing clients to
execute stored procedures on the server. With this mechanism, a disconnection
can never cause a lock timeout, and there is no need for clients to download
the whole database. Each client can upload new data to the server in a very
minimal single round-trip. The write transaction will be executed entirely on
the server, and cannot be interrupted by a disconnection.

A stored procedure is defined by a schema that is used to serialize data for
communication between the client and the server, and a C++ function that will
be executed on the server. For example, this is a stored procedure that inserts
a new city:

.. literalinclude:: ./tutorial/src/tutorial.procedures/city.joedbi
   :language: joedbi
   :caption:

.. literalinclude:: ./tutorial/src/tutorial.procedures/city.joedbc
   :language: joedbc
   :caption:

.. literalinclude:: ./tutorial/src/tutorial.procedures/insert_city.h
   :language: c++
   :caption:

The procedure schema can be used to return values from the server as well. It
can contain any number of tables, and even refer to tables of the main
database. Here is an example of a stored procedure that will count the number
of persons for each city name given as input. It will also return each city id.

.. literalinclude:: ./tutorial/src/tutorial.procedures/population.joedbi
   :language: joedbi
   :caption:

.. literalinclude:: ./tutorial/src/tutorial.procedures/population.joedbc
   :language: joedbc
   :caption:

.. literalinclude:: ./tutorial/src/tutorial.procedures/get_population.h
   :language: c++
   :caption:
