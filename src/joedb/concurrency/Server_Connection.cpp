#include "joedb/concurrency/Server_Connection.h"
#include "joedb/journal/File_Hasher.h"
#include "joedb/error/Disconnection.h"
#include "joedb/ui/Progress_Bar.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>

#define LOG(x) do {if (log) *log << x;} while (false)
#define LOGID(x) do {if (log) *log << get_time_string_of_now() << ' ' << get_session_id() << ": " << x;} while (false)

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 size_t Server_Connection::pread(char *data, size_t size, int64_t offset) const
 ////////////////////////////////////////////////////////////////////////////
 {
  buffer.index = 0;
  buffer.write<char>('r');
  buffer.write<int64_t>(offset);
  buffer.write<int64_t>(offset + int64_t(size));

  Lock<Channel&> lock(channel);
  lock->write(buffer.data, buffer.index);
  lock->read(buffer.data, 9);

  buffer.index = 1;
  const int64_t until = buffer.read<int64_t>();

  if (until < offset || until > offset + int64_t(size))
   throw Disconnection("bad pread size from server");

  const size_t returned_size = size_t(until - offset);

  for (size_t read = 0; read < returned_size;)
   read += lock->read_some(data + read, returned_size - read);

  return returned_size;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::handshake
 ////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  Content_Check content_check
 )
 {
  if (content_check == Content_Check::none)
   return server_checkpoint;

  LOGID("checking_hash... ");

  const int64_t checkpoint = std::min
  (
   server_checkpoint,
   client_journal.get_checkpoint()
  );

  buffer.index = 0;
  buffer.write<char>(content_check == Content_Check::fast ? 'H' : 'I');
  buffer.write<int64_t>(checkpoint);
  buffer.write
  (
   content_check == Content_Check::fast
   ? Journal_Hasher::get_fast_hash(client_journal, checkpoint)
   : Journal_Hasher::get_full_hash(client_journal, checkpoint)
  );

  {
   Lock<Channel&> lock(channel);
   lock->write(buffer.data, buffer.index);
   lock->read(buffer.data, 1);
  }

  LOG(buffer.data[0] << '\n');

  if (buffer.data[0] == 'h')
   content_mismatch();

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Lock_Action lock_action,
  Data_Transfer data_transfer,
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  Lock<Channel&> lock(channel);

  const char pull_type = char('D' + int(lock_action) + 2*int(data_transfer));

  LOGID("pulling, lock = " << int(lock_action) << ", data = " <<
   int(data_transfer) << ", wait = " << double(wait.count()) * 0.001 << "s\n");

  buffer.index = 0;
  buffer.write<char>(pull_type);
  buffer.write<int64_t>(wait.count());
  buffer.write<int64_t>(client_journal.get_checkpoint());
  lock->write(buffer.data, buffer.index);

  buffer.index = 0;
  lock->read(buffer.data, 9);
  {
   const char reply = buffer.read<char>();
   if (reply == 'R')
    throw Disconnection("pull error: server is pull-only, cannot lock");
   else if (reply != pull_type)
    throw Disconnection("pull error: unexpected server reply");
  }
  server_checkpoint = buffer.read<int64_t>();

  if (bool(data_transfer))
  {
   buffer.index = 0;
   const int64_t size = server_checkpoint - client_journal.get_checkpoint();
   Async_Writer writer = client_journal.get_async_tail_writer();
   LOGID("data transfer: ");
   download(writer, lock, size);
   client_journal.soft_checkpoint_at(writer.get_position());
  }
  else
   LOG("no data\n");

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_Connection::push
 ////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  int64_t from,
  int64_t until,
  Unlock_Action unlock_action
 )
 {
  Lock<Channel&> lock(channel);

  const char push_type = char('N' + int(unlock_action));
  buffer.index = 0;
  buffer.write<char>(push_type);
  buffer.write<int64_t>(from);
  buffer.write<int64_t>(until);

  {
   Async_Reader reader = client_journal.get_async_reader(from, until);

   LOGID("pushing(" << push_type << ")... from = " << from << ", until = " << until);
   Progress_Bar progress_bar(reader.get_remaining(), log);

   size_t offset = buffer.index;

   while (offset + reader.get_remaining() > 0)
   {
    const size_t size = reader.read(buffer.data + offset, buffer.size - offset);
    if (reader.is_end_of_file())
     throw Disconnection("push error: unexpected end of file");
    lock->write(buffer.data, size + offset);
    offset = 0;
    progress_bar.print_remaining(reader.get_remaining());
   }
  }

  lock->read(buffer.data, 1);

  if (buffer.data[0] == 'R')
   throw Disconnection("push error: server is pull-only");
  else if (buffer.data[0] == 'C')
   throw Disconnection("push error: conflict");
  else if (buffer.data[0] == 't')
   throw Exception("push error: time out");
  else if (buffer.data[0] != push_type)
   throw Disconnection("push error: unexpected reply");

  server_checkpoint = until;

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  LOGID("releasing lock... ");

  {
   Lock<Channel&> lock(channel);
   buffer.data[0] = 'M';
   lock->write(buffer.data, 1);
   lock->read(buffer.data, 1);
  }

  LOG(buffer.data[0] << '\n');

  if (buffer.data[0] != 'M')
   throw Disconnection("unlock error: unexpected reply");
 }
}

#undef LOGID
#undef LOG
