#include "joedb/concurrency/Server_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Server_File::Server_File(Channel &channel):
 ////////////////////////////////////////////////////////////////////////////
  Server_Connection(channel),
  Generic_File(Open_Mode::read_existing)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t Server_File::pread(char *data, size_t size, int64_t offset)
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
}
