#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/network_integers.h"
#include "joedb/Exception.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::unique_lock<std::mutex> lock(mutex);

  std::cerr << "Pulling... ";

  buffer[0] = 'p';
  const int64_t checkpoint = client_journal.get_checkpoint_position();
  to_network(checkpoint, buffer + 1);
  net::write(socket, net::buffer(buffer, 9));

  net::read(socket, net::buffer(buffer, 17));
  if (buffer[0] != 'p' && from_network(buffer + 1) != checkpoint)
   throw Exception("Could not pull from server");

  const int64_t size = from_network(buffer + 9);

  std::cerr << "size = " << size << "...";

  {
   Writable_Journal::Tail_Writer tail_writer(client_journal);

   for (int64_t read = 0; read < size;)
   {
    const size_t n = socket.read_some(net::buffer(buffer, buffer_size));
    tail_writer.append(buffer, n);
    read += n;
    std::cerr << '.';
   }
  }

  std::cerr << " OK\n";

  if (size < 0)
   throw Exception("Client checkpoint is ahead of server checkpoint");

  return client_journal.get_checkpoint_position();
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::lock_pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  lock();
  return pull(client_journal);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::push_unlock
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_position
 )
 {
  {
   std::unique_lock<std::mutex> lock(mutex);

   Async_Reader reader = client_journal.get_tail_reader(server_position);

   if (reader.get_remaining() > 0)
   {
    buffer[0] = 'P';
    to_network(server_position, buffer + 1);
    to_network(reader.get_remaining(), buffer + 9);

    std::cerr << "pushing " << reader.get_remaining() << " bytes:";

    net::write(socket, net::buffer(buffer, 17));

    while (reader.get_remaining() > 0)
    {
     const size_t size = reader.read(buffer, buffer_size);
     net::write(socket, net::buffer(buffer, size));
     std::cerr << ' ' << size;
    }

    std::cerr << '\n';
   }
  }

  unlock();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::unique_lock<std::mutex> lock(mutex);

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
  std::unique_lock<std::mutex> lock(mutex);

  std::cerr << "Releasing lock... ";

  buffer[0] = 'u';
  net::write(socket, net::buffer(buffer, 1));
  net::read(socket, net::buffer(buffer, 1));
  if (buffer[0] != 'u')
   throw Exception("Could not obtain unlock confirmation from server");

  std::cerr << "OK\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::keep_alive()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::unique_lock<std::mutex> lock(mutex);

  std::cerr << "keep_alive() thread started\n";

  while (!keep_alive_thread_must_stop)
  {
   buffer[0] = 'i';
   net::write(socket, net::buffer(buffer, 1));
   net::read(socket, net::buffer(buffer, 1));

   condition.wait_for(lock, std::chrono::seconds(keep_alive_interval));
  }

  std::cerr << "keep_alive() thread stopping\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 Socket_Construction::Socket_Construction
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
 Server_Connection::Server_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  const char *host_name,
  const char *port_name
 ):
  Socket_Construction(host_name, port_name),
  buffer(new char[buffer_size]),
  keep_alive_thread_must_stop(false),
  keep_alive_thread([this](){keep_alive();})
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::~Server_Connection()
 ////////////////////////////////////////////////////////////////////////////
 {
  {
   std::unique_lock<std::mutex> lock(mutex);
   keep_alive_thread_must_stop = true;
   condition.notify_one();
  }

  keep_alive_thread.join();

  delete[] buffer;
 }
}
