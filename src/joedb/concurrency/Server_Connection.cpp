#include "joedb/concurrency/Server_Connection.h"
#include "joedb/Exception.h"

#include <iostream>
#include <experimental/buffer>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::lock_pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  lock();
  return 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::push_unlock
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_position
 )
 {
  unlock();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Obtaining lock... ";

  buffer[0] = 'l';

  if (net::write(socket, net::buffer(buffer, 1)) != 1)
   throw Exception("Could not send lock command to server");

  if
  (
   net::read(socket, net::buffer(buffer, 1)) != 1 ||
   buffer[0] != 'l'
  )
  {
   throw Exception("Could not obtain lock confirmation from server");
  }

  std::cerr << "OK\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Releasing lock... ";

  buffer[0] = 'u';

  if (net::write(socket, net::buffer(buffer, 1)) != 1)
   throw Exception("Could not send unlock command to server");

  if
  (
   net::read(socket, net::buffer(buffer, 1)) != 1 ||
   buffer[0] != 'u'
  )
   throw Exception("Could not obtain unlock confirmation from server");

  std::cerr << "OK\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::Server_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  const char *host_name,
  const char *port_name
 ):
  socket(io_context)
 {
  std::cerr << "Connecting... ";

  net::ip::tcp::resolver resolver(io_context);
  net::connect
  (
   socket,
   resolver.resolve(host_name, port_name)
  );

  std::cerr << "Waiting for \"joedb\"... ";

  if
  (
   net::read(socket, net::buffer(buffer, 5)) != 5 ||
   buffer[0] != 'j' ||
   buffer[1] != 'o' ||
   buffer[2] != 'e' ||
   buffer[3] != 'd' ||
   buffer[4] != 'b'
  )
  {
   throw Exception("Did not receive \"joedb\" from server");
  }

  std::cerr << "OK.\n";
 }
}
