#include "joedb/concurrency/Server_Connection.h"
#include "joedb/journal/File_Hasher.h"
#include "joedb/Exception.h"
#include "joedb/io/Progress_Bar.h"
#include "joedb/io/get_time_string.h"

#include <iostream>
#include <optional>

#define LOG(x) do {if (log) *log << x;} while (false)
#define LOGID(x) do {if (log) *log << get_time_string_of_now() << ' ' << get_session_id() << ": " << x;} while (false)


namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::unlock(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel_Lock lock(channel);

  LOGID("releasing lock... ");

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
  std::chrono::milliseconds wait,
  char pull_type
 )
 {
  Channel_Lock lock(channel);

  LOGID("pulling(" << pull_type << ")... ");

  buffer.index = 0;
  buffer.write<char>(pull_type);
  const int64_t client_checkpoint = client_journal.get_checkpoint_position();
  buffer.write<int64_t>(client_checkpoint);
  buffer.write<int64_t>(wait.count());
  lock.write(buffer.data, buffer.index);

  buffer.index = 0;
  lock.read(buffer.data, 17);
  if (buffer.read<char>() != pull_type)
   throw Exception("Unexpected server reply");
  const int64_t server_checkpoint = buffer.read<int64_t>();
  const int64_t size = buffer.read<int64_t>();

  LOG("server_checkpoint = " << server_checkpoint << "; size = " << size);

  {
   Writable_Journal::Tail_Writer tail_writer(client_journal);

   std::optional<io::Progress_Bar> progress_bar;
   if (size > buffer.ssize && log)
    progress_bar.emplace(size, *log);

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
    if (progress_bar)
     progress_bar->print(read);
   }

   tail_writer.finish();

   if (!progress_bar)
    LOG(" OK\n");
  }

  if (size < 0)
   throw Exception("Client checkpoint is ahead of server checkpoint");

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::shared_pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait,
  char pull_type
 )
 {
  client_journal.lock_pull();
  const int64_t result = pull(client_journal, wait, pull_type);
  if (pull_type != 'L')
   client_journal.unlock();
  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return shared_pull(client_journal, wait, 'P');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::lock_pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return shared_pull(client_journal, wait, 'L');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::push_until
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_position,
  int64_t until_position,
  bool unlock_after
 )
 {
  Channel_Lock lock(channel);

  Async_Reader reader = client_journal.get_async_reader
  (
   server_position,
   until_position
  );

  const int64_t push_size = reader.get_remaining();

  buffer.index = 0;
  buffer.write<char>(unlock_after ? 'U' : 'p');
  buffer.write<int64_t>(server_position);
  buffer.write<int64_t>(push_size);

  LOGID("pushing(U)... position = " << server_position << ", size = " << push_size);

  {
   size_t offset = buffer.index;

   std::optional<io::Progress_Bar> progress_bar;
   if (reader.get_remaining() > buffer.ssize && log)
    progress_bar.emplace(reader.get_remaining(), *log);

   int64_t written = 0;

   while (offset + reader.get_remaining() > 0)
   {
    const size_t size = reader.read(buffer.data + offset, buffer.size - offset);
    lock.write(buffer.data, size + offset);
    written += int64_t(size + offset);
    offset = 0;
    if (progress_bar)
     progress_bar->print(written);
   }

   if (!progress_bar)
    LOG(" done\n");
  }

  lock.read(buffer.data, 1);

  if (buffer.data[0] == 'C')
   throw Exception("Conflict: push failed");
  else if (buffer.data[0] == 'R')
   throw Exception("Server is read-only: push failed");
  else if (buffer.data[0] == 't')
   throw Exception("Timeout: push failed");
  else if (buffer.data[0] != 'U')
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

  LOGID("checking_hash... ");

  buffer.index = 0;
  buffer.write<char>('H');

  const int64_t checkpoint = std::min
  (
   server_checkpoint,
   client_journal.get_checkpoint_position()
  );

  buffer.write(checkpoint);
  buffer.write(Journal_Hasher::get_hash(client_journal, checkpoint));

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
    condition.wait_for
    (
     lock,
     std::chrono::seconds(keep_alive_interval_seconds)
    );

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
 int64_t Server_Connection::handshake
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  bool content_check
 )
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

  if (server_version < 13)
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

  if (keep_alive_interval_seconds > 0)
  {
   keep_alive_thread_must_stop = false;
   keep_alive_thread = std::thread([this](){keep_alive();});
  }

  if (content_check)
   if (!check_matching_content(client_journal, server_checkpoint))
    content_mismatch();

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Connection::Server_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  Channel &channel,
  std::ostream *log,
  int keep_alive_interval_seconds
 ):
  channel(channel),
  log(log),
  keep_alive_interval_seconds(keep_alive_interval_seconds),
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
