#include "joedb/concurrency/Server_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Server_File::Server_File
 ////////////////////////////////////////////////////////////////////////////
 (
  Channel &channel,
  std::ostream *log
 ):
  Server_Client(channel, log),
  Generic_File(Open_Mode::read_existing),
  server_checkpoint(connect())
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t Server_File::pread(char *data, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  Server_Client::buffer.index = 0;
  Server_Client::buffer.write<char>('r');
  Server_Client::buffer.write<int64_t>(offset);
  Server_Client::buffer.write<int64_t>(size);

  Channel_Lock lock(channel);
  lock.write(Server_Client::buffer.data, Server_Client::buffer.index);
  lock.read(Server_Client::buffer.data, 9);

  Server_Client::buffer.index = 1;
  const int64_t returned_size = Server_Client::buffer.read<int64_t>();

  for (int64_t read = 0; read < returned_size;)
   read += int64_t(lock.read_some(data + read, returned_size - read));

  return returned_size;
 }
}
