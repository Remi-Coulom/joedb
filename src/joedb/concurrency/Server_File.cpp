#include "joedb/concurrency/Server_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 size_t Server_File::remote_pread(char *data, size_t size, int64_t offset) const
 ////////////////////////////////////////////////////////////////////////////
 {
  Server_Connection::buffer.index = 0;
  Server_Connection::buffer.write<char>('r');
  Server_Connection::buffer.write<int64_t>(offset);
  Server_Connection::buffer.write<uint64_t>(size);

  Channel_Lock lock(channel);
  lock.write(Server_Connection::buffer.data, Server_Connection::buffer.index);
  lock.read(Server_Connection::buffer.data, 9);

  Server_Connection::buffer.index = 1;
  const size_t returned_size = size_t(Server_Connection::buffer.read<int64_t>());

  if (returned_size > size)
   throw Exception("bad pread size from server");

  for (size_t read = 0; read < returned_size;)
   read += lock.read_some(data + read, returned_size - read);

  return returned_size;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_File::write_to_body_error()
 ////////////////////////////////////////////////////////////////////////////
 {
  throw Exception("Cannot write to Server_File body");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_File::write_checkpoint()
 ////////////////////////////////////////////////////////////////////////////
 {
  head.set_position(0);
  head.write<int64_t>(server_checkpoint);
  head.write<int64_t>(server_checkpoint);
  head.flush();
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_File::Server_File(Channel &channel):
 ////////////////////////////////////////////////////////////////////////////
  Server_Connection(channel),
  Buffered_File(Open_Mode::write_existing),
  tail_offset(server_checkpoint)
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
  std::chrono::milliseconds wait,
  char pull_type
 )
 {
  if (tail.get_size() > 0)
   throw Exception("Server_File: pulling with non-empty tail");

  Server_Connection::pull(nullptr, wait, pull_type);
  write_checkpoint();
  tail_offset = server_checkpoint;

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::handshake
 ////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  bool content_check
 )
 {
  if (&client_journal.get_file() != this)
   throw Exception("Server_File: wrong file");
  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return pull(wait, 'i');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::lock_pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return pull(wait, 'l');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::push_until
 ////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &client_journal,
  const int64_t server_position,
  const int64_t until_position,
  const bool unlock_after
 )
 {
  Server_Connection::push_until
  (
   client_journal,
   server_position,
   until_position,
   unlock_after
  );

  if (server_checkpoint == get_size())
  {
   tail_offset = server_checkpoint;
   tail.resize(0);
  }
  else
   throw Exception("Server_File could not truncate tail after push");

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t Server_File::pread(char *data, size_t size, int64_t offset) const
 ////////////////////////////////////////////////////////////////////////////
 {
  if (offset < Header::ssize)
   return head.pread(data, size, offset);

  if (offset < tail_offset)
   return remote_pread(data, size, offset);

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
