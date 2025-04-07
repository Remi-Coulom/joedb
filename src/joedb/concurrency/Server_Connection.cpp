#include "joedb/concurrency/Server_Connection.h"
#include "joedb/journal/File_Hasher.h"
#include "joedb/error/Exception.h"
#include "joedb/ui/Progress_Bar.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>
#include <optional>

#define LOG(x) do {if (log) *log << x;} while (false)
#define LOGID(x) do {if (log) *log << get_time_string_of_now() << ' ' << get_session_id() << ": " << x;} while (false)

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::unlock()
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
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait,
  char pull_type,
  bool has_data
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
  lock.read(buffer.data, 9);
  {
   const char reply = buffer.read<char>();
   if (reply == 'R')
    throw Exception("Server is pull-only");
   else if (reply != pull_type)
    throw Exception("Unexpected server reply");
  }
  server_checkpoint = buffer.read<int64_t>();

  if (server_checkpoint < client_checkpoint)
   throw Exception("Client checkpoint is ahead of server checkpoint");

  if (has_data)
  {
   buffer.index = 0;
   lock.read(buffer.data, 8);
   const int64_t size = buffer.read<int64_t>();
   client_journal.flush(); // ??? necessary ???
   const int64_t old_position = client_journal.get_position();
   Async_Writer writer = client_journal.get_async_tail_writer();
   download(writer, lock, size);
   client_journal.set_position(writer.get_position());
   client_journal.default_checkpoint();
   client_journal.set_position(old_position);
  }
  else
   LOG("no data\n");

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return pull(client_journal, wait, 'P', true);
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::lock_pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return pull(client_journal, wait, 'L', true);
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

   std::optional<Progress_Bar> progress_bar;
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
   throw Exception("Server is pull-only: push failed");
  else if (buffer.data[0] == 't')
   throw Exception("Timeout: push failed");
  else if (buffer.data[0] != 'U')
   throw Exception("Unexpected server reply");

  server_checkpoint = server_position + push_size;
  return server_checkpoint;
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
 int64_t Server_Connection::handshake
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  bool content_check
 )
 {
  if (content_check)
   if (!check_matching_content(client_journal, server_checkpoint))
    content_mismatch();

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Server_Connection::is_pullonly() const
 ////////////////////////////////////////////////////////////////////////////
 {
  return pullonly_server;
 }
}

#undef LOGID
#undef LOG
