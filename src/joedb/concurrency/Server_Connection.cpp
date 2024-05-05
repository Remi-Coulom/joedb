#include "joedb/concurrency/Server_Connection.h"
#include "joedb/Exception.h"

#include <iostream>

#define LOG(x) do {if (log) *log << x;} while (false)

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::unlock(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel_Lock lock(channel);

  LOG(get_session_id() << ": releasing lock... ");

  buffer.data[0] = 'u';
  lock.write(buffer.data, 1);
  lock.read(buffer.data, 1);

  if (buffer.data[0] == 'u')
   LOG("OK\n");
  else if (buffer.data[0] == 't')
   LOG("The lock had timed out\n");
  else
   throw Exception("Unexpected server reply");

  client_journal.unlock();
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  char pull_type
 )
 {
  Channel_Lock lock(channel);

  LOG(get_session_id() << ": pulling(" << pull_type << ")... ");

  buffer.index = 0;
  buffer.write<char>(pull_type);
  const int64_t client_checkpoint = client_journal.get_checkpoint_position();
  buffer.write<int64_t>(client_checkpoint);
  lock.write(buffer.data, buffer.index);

  buffer.index = 0;
  lock.read(buffer.data, 17);
  if (buffer.read<char>() != pull_type)
   throw Exception("Unexpected server reply");
  const int64_t server_checkpoint = buffer.read<int64_t>();
  const int64_t size = buffer.read<int64_t>();

  LOG("checkpoint = " << server_checkpoint << "; size = " << size << "...");

  {
   Writable_Journal::Tail_Writer tail_writer(client_journal);

   for (int64_t read = 0; read < size;)
   {
    const int64_t remaining = size - read;
    const size_t read_size = size_t
    (
     std::min(int64_t(buffer.size), remaining)
    );
    const size_t n = lock.read_some(buffer.data, read_size);
    tail_writer.append(buffer.data, n);
    read += int64_t(n);
    LOG('.');
   }

   tail_writer.finish();
  }

  LOG(" OK\n");

  if (size < 0)
   throw Exception("Client checkpoint is ahead of server checkpoint");

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::shared_pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  char pull_type
 )
 {
  client_journal.lock_pull();
  const int64_t result = pull(client_journal, pull_type);
  if (pull_type == 'P')
   client_journal.unlock();
  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return shared_pull(client_journal, 'P');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::lock_pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return shared_pull(client_journal, 'L');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::push
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_position,
  bool unlock_after
 )
 {
  Channel_Lock lock(channel);

  Async_Reader reader = client_journal.get_async_tail_reader(server_position);

  buffer.index = 0;
  buffer.write<char>(unlock_after ? 'U' : 'p');
  buffer.write<int64_t>(server_position);
  buffer.write<int64_t>(reader.get_remaining());

  const int64_t push_size = reader.get_remaining();

  LOG(get_session_id() << ": pushing(U)... size = " << push_size << "... ");

  {
   size_t offset = buffer.index;

   while (offset + reader.get_remaining() > 0)
   {
    const size_t size = reader.read(buffer.data + offset, buffer.size - offset);
    lock.write(buffer.data, size + offset);
    offset = 0;
    LOG(size << ' ');
   }
  }

  lock.read(buffer.data, 1);

  if (buffer.data[0] == 'U')
   LOG("OK\n");
  else if (buffer.data[0] == 'C')
   throw Exception("Conflict: push failed");
  else if (buffer.data[0] == 'R')
   throw Exception("Server is read-only: push failed");
  else if (buffer.data[0] == 't')
   throw Exception("Timeout: push failed");
  else
   throw Exception("Unexpected server reply");

  if (unlock_after)
   client_journal.unlock();

  return client_journal.get_checkpoint_position();
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Server_Connection::check_matching_content
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal, 
  int64_t server_checkpoint
 )
 {
  Channel_Lock lock(channel);

  LOG(get_session_id() << ": checking_hash... ");

  buffer.index = 0;
  buffer.write<char>('H');

  const int64_t checkpoint = std::min
  (
   server_checkpoint,
   client_journal.get_checkpoint_position()
  );

  buffer.write<int64_t>(checkpoint);

  SHA_256::Hash hash = client_journal.get_hash(checkpoint);
  for (uint32_t i = 0; i < 8; i++)
   buffer.write<uint32_t>(hash[i]);

  lock.write(buffer.data, buffer.index);
  lock.read(buffer.data, 1);

  const bool result = (buffer.data[0] == 'H');

  if (result)
   LOG("OK\n");
  else
   LOG("Error\n");

  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::ping(Channel_Lock &lock)
 ////////////////////////////////////////////////////////////////////////////
 {
  buffer.data[0] = 'i';
  lock.write(buffer.data, 1);
  lock.read(buffer.data, 1);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::ping()
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel_Lock lock(channel);
  ping(lock);
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

    ping(lock);
   }
  }
  catch(...)
  {
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::handshake(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  LOG("Connecting... ");

  buffer.index = 0;
  buffer.write<char>('j');
  buffer.write<char>('o');
  buffer.write<char>('e');
  buffer.write<char>('d');
  buffer.write<char>('b');
  buffer.write<int64_t>(client_version);

  {
   Channel_Lock lock(channel);
   lock.write(buffer.data, buffer.index);
   LOG("Waiting for \"joedb\"... ");
   lock.read(buffer.data, 5 + 8 + 8 + 8 + 1);
  }

  buffer.index = 0;

  if
  (
   buffer.read<char>() != 'j' ||
   buffer.read<char>() != 'o' ||
   buffer.read<char>() != 'e' ||
   buffer.read<char>() != 'd' ||
   buffer.read<char>() != 'b'
  )
  {
   throw Exception("Did not receive \"joedb\" from server");
  }

  const int64_t server_version = buffer.read<int64_t>();

  if (server_version == 0)
   throw Exception("Client version rejected by server");

  LOG("server_version = " << server_version << ". ");

  if (server_version < 9)
   throw Exception("Unsupported server version");

  session_id = buffer.read<int64_t>();
  const int64_t server_checkpoint = buffer.read<int64_t>();
  const char mode = buffer.read<char>();

  if (mode == 'R')
   pullonly_server = true;
  else if (mode == 'W')
   pullonly_server = false;
  else
   throw Exception("Unexpected server mode");

  LOG
  (
   "session_id = " << session_id <<
   "; server_checkpoint = " << server_checkpoint <<
   "; mode = " << mode <<
   ". OK.\n"
  );

  keep_alive_thread_must_stop = false;
  keep_alive_thread = std::thread([this](){keep_alive();});

  if (!check_matching_content(client_journal, server_checkpoint))
   content_mismatch();

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::Server_Connection(Channel &channel, std::ostream *log):
 ////////////////////////////////////////////////////////////////////////////
  channel(channel),
  log(log),
  session_id(-1),
  pullonly_server(false)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 Connection *Server_Connection::get_push_connection()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (pullonly_server)
   return nullptr;
  else
   return this;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::~Server_Connection()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   Channel_Lock lock(channel);
   keep_alive_thread_must_stop = true;
   buffer.data[0] = 'Q';
   lock.write(buffer.data, 1);
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
