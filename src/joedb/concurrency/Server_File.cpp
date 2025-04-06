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
  head.set_position(Readonly_Journal::checkpoint_offset);
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
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait,
  char pull_type
 )
 {
  if (tail.get_size() > 0)
   throw Exception("Server_File: pulling with non-empty tail");

  int64_t result = Server_Connection::pull(client_journal, wait, pull_type, false);
  write_checkpoint();
  client_journal.pull();
  tail_offset = result;

  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::handshake
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  bool content_check
 )
 {
  if (!client_journal.is_same_file(*this))
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
  return pull(client_journal, wait, 'i');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::lock_pull
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return pull(client_journal, wait, 'l');
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::push_until
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
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
  if (offset < Readonly_Journal::header_size)
   return head.pread(data, size, offset);

  if (offset < tail_offset)
   return remote_pread(data, size, offset);

  return tail.pread(data, size, offset - tail_offset);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_File::pwrite(const char *data, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (offset < Readonly_Journal::header_size)
  {
   if (offset + size > Readonly_Journal::header_size)
    write_to_body_error();
   else
    head.pwrite(data, size, offset);
  }
  else if (offset >= tail_offset)
   tail.pwrite(data, size, offset - tail_offset);
  else
   write_to_body_error();
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Server_File::read_blob_data(Blob blob) const
 ////////////////////////////////////////////////////////////////////////////
 {
  if (blob.get_position() >= tail_offset)
   return tail.read_blob_data(Blob{blob.get_position() - tail_offset});
  else
  {
   Channel_Lock lock(channel);

   Server_Connection::buffer.index = 0;
   Server_Connection::buffer.write<char>('b');
   Server_Connection::buffer.write<int64_t>(blob.get_position());
   lock.write(Server_Connection::buffer.data, Server_Connection::buffer.index);

   lock.read(Server_Connection::buffer.data, 9);
   Server_Connection::buffer.index = 1;
   const int64_t size = Server_Connection::buffer.read<int64_t>();

   Memory_File file;
   file.resize(size_t(size));
   joedb::Async_Writer writer(file, 0);
   download(writer, lock, size);

   return file.move_data();
  }
 }
}
