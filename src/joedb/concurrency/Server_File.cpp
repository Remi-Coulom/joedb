#include "joedb/concurrency/Server_File.h"

#include <ostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 size_t Server_File::remote_pread(char *data, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  Server_Connection::buffer.index = 0;
  Server_Connection::buffer.write<char>('r');
  Server_Connection::buffer.write<int64_t>(offset);
  Server_Connection::buffer.write<int64_t>(size);

  Channel_Lock lock(channel);
  lock.write(Server_Connection::buffer.data, Server_Connection::buffer.index);
  lock.read(Server_Connection::buffer.data, 9);

  Server_Connection::buffer.index = 1;
  const int64_t returned_size = Server_Connection::buffer.read<int64_t>();

  for (int64_t read = 0; read < returned_size;)
   read += int64_t(lock.read_some(data + read, returned_size - read));

  return returned_size;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_File::write_to_body_error()
 ////////////////////////////////////////////////////////////////////////////
 {
  throw Runtime_Error("Cannot write to Server_File body");
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_File::Server_File(Channel &channel):
 ////////////////////////////////////////////////////////////////////////////
  Server_Connection(channel),
  Generic_File(Open_Mode::write_existing),
  tail_offset(server_checkpoint)
 {
  {
   Writable_Journal journal(head);
  }
  head.set_position(Readonly_Journal::checkpoint_offset);
  head.write<int64_t>(server_checkpoint);
  head.write<int64_t>(server_checkpoint);
  head.write<int64_t>(server_checkpoint);
  head.write<int64_t>(server_checkpoint);
  head.flush();
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Server_File::push_until
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_position,
  int64_t until_position,
  bool unlock_after
 )
 {
  const int64_t new_server_checkpoint = Server_Connection::push_until
  (
   client_journal,
   server_position,
   until_position,
   unlock_after
  );

  if (new_server_checkpoint == get_size())
  {
   tail_offset = new_server_checkpoint;
   tail.resize(0);
   if (log)
    *log << "Cleared tail\n";
  }

  return new_server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t Server_File::pread(char *data, size_t size, int64_t offset)
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
 std::string Server_File::read_blob_data(Blob blob)
 ////////////////////////////////////////////////////////////////////////////
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
  joedb::Async_Writer writer(file, 0);
  download(writer, lock, size);

  return file.move_data();
 }
}
