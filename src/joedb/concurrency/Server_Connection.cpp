#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/network_integers.h"
#include "joedb/Exception.h"

#include <iostream>
#include <experimental/buffer>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Pulling... ";

  buffer[0] = 'p';
  const int64_t checkpoint = client_journal.get_checkpoint_position();
  to_network(checkpoint, buffer + 1);
  net::write(socket, net::buffer(buffer, 9));

  net::read(socket, net::buffer(buffer, 17));
  if (buffer[0] != 'p' && from_network(buffer + 1) != checkpoint)
   throw Exception("Could not pull from server");

  const int64_t size = from_network(buffer + 9);

  std::cerr << "size = " << size << "... ";

  {
   Writable_Journal::Tail_Writer tail_writer(client_journal);

   for (int64_t read = 0; read < size;)
   {
    const size_t n = socket.read_some(net::buffer(buffer, buffer_size));
    tail_writer.append(buffer, n);
    read += n;
   }
  }

  std::cerr << "OK\n";

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
  net::write(socket, net::buffer(buffer, 1));
  net::read(socket, net::buffer(buffer, 1));
  if (buffer[0] != 'l')
   throw Exception("Could not obtain lock confirmation from server");

  std::cerr << "OK\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Releasing lock... ";

  buffer[0] = 'u';
  net::write(socket, net::buffer(buffer, 1));

  std::cerr << "OK\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::Server_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  const char *host_name,
  const char *port_name
 ):
  socket(io_context),
  buffer(new char[buffer_size])
 {
  std::cerr << "Connecting... ";

  net::ip::tcp::resolver resolver(io_context);
  net::connect
  (
   socket,
   resolver.resolve(host_name, port_name)
  );

  std::cerr << "Waiting for \"joedb\"... ";

  char buffer[5];

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

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::~Server_Connection()
 ////////////////////////////////////////////////////////////////////////////
 {
  delete[] buffer;
 }
}
