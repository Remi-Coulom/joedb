#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/network_integers.h"
#include "joedb/Exception.h"

#include <iostream>

#define LOG(x) do {if (log) *log << x;} while (false)

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (client_journal.is_shared())
   client_journal.exclusive_lock();

  Channel_Lock lock(channel);

  LOG(get_session_id() << ": obtaining lock... ");

  buffer[0] = 'l';
  lock.write(buffer.data(), 1);
  lock.read(buffer.data(), 1);
  if (buffer[0] != 'l')
   throw Exception("Unexpected server reply");

  LOG("OK\n");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (client_journal.is_shared())
   client_journal.unlock();

  Channel_Lock lock(channel);

  LOG(get_session_id() << ": releasing lock... ");

  buffer[0] = 'u';
  lock.write(buffer.data(), 1);
  lock.read(buffer.data(), 1);

  if (buffer[0] == 'u')
   LOG("OK\n");
  else if (buffer[0] == 't')
   LOG("The lock had timed out\n");
  else
   throw Exception("Unexpected server reply");
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull(char pull_type)
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel_Lock lock(channel);

  LOG(get_session_id() << ": pulling(" << pull_type << ")... ");

  buffer[0] = pull_type;
  const int64_t checkpoint = client_journal.get_checkpoint_position();
  to_network(checkpoint, buffer.data() + 1);
  lock.write(buffer.data(), 9);

  lock.read(buffer.data(), 17);
  if (buffer[0] != pull_type && from_network(buffer.data() + 1) != checkpoint)
   throw Exception("Could not pull from server");

  const int64_t size = from_network(buffer.data() + 9);

  LOG("size = " << size << "...");

  {
   Writable_Journal::Tail_Writer tail_writer(client_journal);

   for (int64_t read = 0; read < size;)
   {
    const int64_t remaining = size - read;
    size_t read_size = size_t(remaining);
    if (read_size > buffer_size)
     read_size = buffer_size;
    const size_t n = lock.read_some(buffer.data(), read_size);
    tail_writer.append(buffer.data(), n);
    read += n;
    LOG('.');
   }

   tail_writer.finish();
  }

  LOG(" OK\n");

  if (size < 0)
   throw Exception("Client checkpoint is ahead of server checkpoint");

  return client_journal.get_checkpoint_position();
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::shared_pull(char pull_type)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (client_journal.is_shared())
  {
   client_journal.exclusive_lock();
   client_journal.refresh_checkpoint();
   int64_t result = pull(pull_type);
   if (pull_type == 'P')
    client_journal.unlock();
   return result;
  }
  else
   return pull(pull_type);
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull()
 ////////////////////////////////////////////////////////////////////////////
 {
  return shared_pull('P');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::lock_pull()
 ////////////////////////////////////////////////////////////////////////////
 {
  return shared_pull('L');
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::push(int64_t server_position, bool unlock_after)
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel_Lock lock(channel);

  Async_Reader reader = client_journal.get_tail_reader(server_position);

  buffer[0] = unlock_after ? 'U' : 'p';
  to_network(server_position, buffer.data() + 1);
  to_network(reader.get_remaining(), buffer.data() + 9);

  const int64_t push_size = reader.get_remaining();

  LOG(get_session_id() << ": pushing(U)... size = " << push_size << "... ");

  lock.write(buffer.data(), 17);

  while (reader.get_remaining() > 0)
  {
   const size_t size = reader.read(buffer.data(), buffer_size);
   lock.write(buffer.data(), size);
   LOG(size << ' ');
  }

  lock.read(buffer.data(), 1);

  if (unlock_after && client_journal.is_shared())
   client_journal.unlock();

  if (buffer[0] == 'U')
   LOG("OK\n");
  else if (buffer[0] == 'C')
   throw Exception("Conflict: push failed");
  else
   throw Exception("Unexpected server reply");
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Server_Connection::check_matching_content(int64_t server_checkpoint)
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel_Lock lock(channel);

  LOG(get_session_id() << ": checking_hash... ");

  buffer[0] = 'H';

  const int64_t checkpoint = std::min
  (
   server_checkpoint,
   client_journal.get_checkpoint_position()
  );

  to_network(checkpoint, buffer.data() + 1);

  SHA_256::Hash hash = client_journal.get_hash(checkpoint);
  for (uint32_t i = 0; i < 8; i++)
   uint32_to_network(hash[i], buffer.data() + 9 + 4 * i);

  lock.write(buffer.data(), 41);
  lock.read(buffer.data(), 1);

  const bool result = (buffer[0] == 'H');

  if (result)
   LOG("OK\n");
  else
   LOG("Error\n");

  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::keep_alive()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   Channel_Lock lock(channel);

   while (!keep_alive_thread_must_stop)
   {
    condition.wait_for(lock, std::chrono::seconds(keep_alive_interval));

    if (keep_alive_thread_must_stop)
     break;

    buffer[0] = 'i';

    lock.write(buffer.data(), 1);
    lock.read(buffer.data(), 1);
   }
  }
  catch(...)
  {
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::handshake()
 ////////////////////////////////////////////////////////////////////////////
 {
  LOG("Connecting... ");

  buffer[0] = 'j';
  buffer[1] = 'o';
  buffer[2] = 'e';
  buffer[3] = 'd';
  buffer[4] = 'b';

  const int64_t client_version = 6;
  to_network(client_version, buffer.data() + 5);

  {
   Channel_Lock lock(channel);
   lock.write(buffer.data(), 5 + 8);
   LOG("Waiting for \"joedb\"... ");
   lock.read(buffer.data(), 5 + 8 + 8 + 8);
  }

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

  const int64_t server_version = from_network(buffer.data() + 5);

  if (server_version == 0)
   throw Exception("Client version rejected by server");

  LOG("server_version = " << server_version << ". ");

  if (server_version < client_version)
   throw Exception("Unsupported server version");

  session_id = from_network(buffer.data() + 5 + 8);
  const int64_t server_checkpoint = from_network(buffer.data() + 5 + 8 + 8);

  LOG
  (
   "session_id = " << session_id <<
   "; server_checkpoint = " << server_checkpoint <<
   ". OK.\n"
  );

  keep_alive_thread_must_stop = false;
  keep_alive_thread = std::thread([this](){keep_alive();});

  if (!check_matching_content(server_checkpoint))
   content_mismatch();

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::Server_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  Channel &channel,
  std::ostream *log
 ):
  Connection(client_journal),
  channel(channel),
  log(log),
  buffer(buffer_size),
  session_id(-1)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::~Server_Connection()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   Channel_Lock lock(channel);
   keep_alive_thread_must_stop = true;
   buffer[0] = 'Q';
   lock.write(buffer.data(), 1);
  }
  catch (...)
  {
   postpone_exception("Could not write to server");
  }

  try
  {
   condition.notify_one();
   if (keep_alive_thread.joinable())
    keep_alive_thread.join();
  }
  catch (...)
  {
   postpone_exception("Could not join keep-alive thread");
  }
 }
}

#undef LOG
