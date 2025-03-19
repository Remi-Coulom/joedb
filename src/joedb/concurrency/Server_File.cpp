#include "joedb/concurrency/Server_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_File::write_to_body_error()
 ////////////////////////////////////////////////////////////////////////////
 {
  throw Runtime_Error("Cannot write to Server_File body");
 }

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
 Server_File::Server_File(Channel &channel):
 ////////////////////////////////////////////////////////////////////////////
  Server_Connection(channel),
  Generic_File(Open_Mode::write_existing),
  tail_offset(server_checkpoint)
 {
  head.resize(Readonly_Journal::header_size);

  const size_t size = remote_pread
  (
   head.get_data().data(),
   Readonly_Journal::header_size,
   0
  );

  if (size < Readonly_Journal::header_size)
   throw Runtime_Error("error reading remote file head");
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
}
