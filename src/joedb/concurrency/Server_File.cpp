#include "joedb/concurrency/Server_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Server_File::Server_File(Server_Client &client):
 ////////////////////////////////////////////////////////////////////////////
 Generic_File(Open_Mode::read_existing),  client(client)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t Server_File::pread(char *data, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  client.buffer.index = 0;
  client.buffer.write<char>('r');
  client.buffer.write<int64_t>(offset);
  client.buffer.write<int64_t>(size);

  Channel_Lock lock(client.channel);
  lock.write(client.buffer.data, client.buffer.index);
  lock.read(client.buffer.data, 9);

  client.buffer.index = 1;
  const int64_t returned_size = client.buffer.read<int64_t>();

  for (int64_t read = 0; read < returned_size;)
   read += int64_t(lock.read_some(data + read, returned_size - read));

  return returned_size;
 }
}
