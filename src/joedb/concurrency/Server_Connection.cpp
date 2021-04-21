#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/network_integers.h"
#include "joedb/Exception.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  char pull_type
 )
 {
  std::unique_lock<std::mutex> lock(mutex);

  std::cerr << "Pulling(" << pull_type << ")... ";

  buffer[0] = pull_type;
  const int64_t checkpoint = client_journal.get_checkpoint_position();
  to_network(checkpoint, buffer + 1);
  channel.write(buffer, 9);

  channel.read(buffer, 17);
  if (buffer[0] != pull_type && from_network(buffer + 1) != checkpoint)
   throw Exception("Could not pull from server");

  const int64_t size = from_network(buffer + 9);

  std::cerr << "size = " << size << "...";

  {
   Writable_Journal::Tail_Writer tail_writer(client_journal);

   for (int64_t read = 0; read < size;)
   {
    const int64_t remaining = size - read;
    size_t read_size = size_t(remaining);
    if (read_size > buffer_size)
     read_size = buffer_size;
    const size_t n = channel.read_some(buffer, read_size);
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
 int64_t Server_Connection::pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return pull(client_journal, 'P');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::lock_pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return pull(client_journal, 'L');
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::push_unlock
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_position
 )
 {
  std::unique_lock<std::mutex> lock(mutex);

  Async_Reader reader = client_journal.get_tail_reader(server_position);

  buffer[0] = 'U';
  to_network(server_position, buffer + 1);
  to_network(reader.get_remaining(), buffer + 9);

  const int64_t push_size = reader.get_remaining();

  std::cerr << "pushing " << push_size << " bytes: ";

  channel.write(buffer, 17);

  while (reader.get_remaining() > 0)
  {
   const size_t size = reader.read(buffer, buffer_size);
   channel.write(buffer, size);
   std::cerr << size << ' ';
  }

  channel.read(buffer, 1);

  if (buffer[0] == 'U')
   std::cerr << "OK\n";
  else if (buffer[0] == 'C')
   throw Exception("Conflict: push failed");
  else
   throw Exception("Unexpected server reply");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::unique_lock<std::mutex> lock(mutex);

  std::cerr << "Obtaining lock... ";

  buffer[0] = 'l';
  channel.write(buffer, 1);
  channel.read(buffer, 1);
  if (buffer[0] != 'l')
   throw Exception("Unexpected server reply");

  std::cerr << "OK\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::unique_lock<std::mutex> lock(mutex);

  std::cerr << "Releasing lock... ";

  buffer[0] = 'u';
  channel.write(buffer, 1);
  channel.read(buffer, 1);

  if (buffer[0] == 'u')
   std::cerr << "OK\n";
  else if (buffer[0] == 't')
   std::cerr << "The lock had timed out\n";
  else
   throw Exception("Unexpected server reply");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::keep_alive()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::unique_lock<std::mutex> lock(mutex);

  std::cerr << "keep_alive() thread started\n";

  try
  {
   while (!keep_alive_thread_must_stop)
   {
    condition.wait_for(lock, std::chrono::seconds(keep_alive_interval));

    if (keep_alive_thread_must_stop)
     break;

    buffer[0] = 'i';

    channel.write(buffer, 1);
    channel.read(buffer, 1);
   }
  }
  catch(...)
  {
  }

  std::cerr << "keep_alive() thread stopping\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Handshake::Server_Handshake(Channel &channel): channel(channel)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "Connecting... ";

  char buffer[5 + 8];

  buffer[0] = 'j';
  buffer[1] = 'o';
  buffer[2] = 'e';
  buffer[3] = 'd';
  buffer[4] = 'b';

  const int64_t client_version = 2;
  to_network(client_version, buffer + 5);

  channel.write(buffer, 5 + 8);
  channel.read(buffer, 5 + 8);

  std::cerr << "Waiting for \"joedb\"... ";

  if
  (
   buffer[0] != 'j' ||
   buffer[1] != 'o' ||
   buffer[2] != 'e' ||
   buffer[3] != 'd' ||
   buffer[4] != 'b'
  )
  {
   throw Exception("Did not receive \"joedb\" from server");
  }

  const int64_t server_version = from_network(buffer + 5);

  if (server_version == 0)
   throw Exception("Client version rejected by server");

  std::cerr << "server_version = " << server_version << ". OK.\n";

  if (server_version < 2)
   throw Exception("Unsupported server version");
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::Server_Connection(Channel &channel):
 ////////////////////////////////////////////////////////////////////////////
  Server_Handshake(channel),
  buffer(new char[buffer_size]),
  keep_alive_thread_must_stop(false),
  keep_alive_thread([this](){keep_alive();})
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::~Server_Connection()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   buffer[0] = 'Q';
   channel.write(buffer, 1);
  }
  catch(...)
  {
  }

  {
   std::unique_lock<std::mutex> lock(mutex);
   keep_alive_thread_must_stop = true;
   condition.notify_one();
  }

  keep_alive_thread.join();

  delete[] buffer;
 }
}
