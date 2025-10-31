#include "joedb/concurrency/Server_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_File::write_to_body_error()
 ////////////////////////////////////////////////////////////////////////////
 {
  throw Disconnection("Cannot write to Server_File body");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_File::write_checkpoint()
 ////////////////////////////////////////////////////////////////////////////
 {
  head.pwrite((const char *)&connection->server_checkpoint, 8, 0);
  head.pwrite((const char *)&connection->server_checkpoint, 8, 8);
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_File::Server_File(const Connector &connector, joedb::Logger *logger):
 ////////////////////////////////////////////////////////////////////////////
  Robust_Connection(connector, logger),
  Abstract_File(Open_Mode::write_existing),
  tail_offset(connection->server_checkpoint)
 {
  {
   Writable_Journal journal(head);
  }
  write_checkpoint();
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Lock_Action lock_action,
  Data_Transfer data_transfer,
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  if (tail.get_size() > 0)
   throw Disconnection("Server_File: pulling with non-empty tail");

  Robust_Connection::pull
  (
   lock_action,
   Data_Transfer::without_data,
   client_journal,
   wait
  );

  write_checkpoint();
  client_journal.pull();
  tail_offset = connection->server_checkpoint;

  return connection->server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::handshake
 ////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  Content_Check content_check
 )
 {
  if (&client_journal.get_file() != this)
   throw Disconnection("Server_File: wrong file");
  return connection->server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::push
 ////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  const int64_t server_position,
  const int64_t until_position,
  const Unlock_Action unlock_action
 )
 {
  Robust_Connection::push
  (
   client_journal,
   server_position,
   until_position,
   unlock_action
  );

  if (connection->server_checkpoint == get_size())
  {
   tail_offset = connection->server_checkpoint;
   tail.resize(0);
  }
  else
   throw Disconnection("Server_File could not truncate tail after push");

  return connection->server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t Server_File::pread(char *data, size_t size, int64_t offset) const
 ////////////////////////////////////////////////////////////////////////////
 {
  if (offset < Header::ssize)
   return head.pread(data, size, offset);

  if (offset < tail_offset)
   return Robust_Connection::pread(data, size, offset);

  return tail.pread(data, size, offset - tail_offset);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_File::pwrite(const char *data, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (offset < Header::ssize)
  {
   if (offset + size > Header::ssize)
    write_to_body_error();
   else
    head.pwrite(data, size, offset);
  }
  else if (offset >= tail_offset)
   tail.pwrite(data, size, offset - tail_offset);
  else
   write_to_body_error();
 }
}
