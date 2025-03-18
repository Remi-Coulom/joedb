#include "joedb/concurrency/Server_Blob_Client.h"
#include "joedb/journal/Memory_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Server_Blob_Client::Server_Blob_Client(Channel &channel, std::ostream *log):
 ////////////////////////////////////////////////////////////////////////////
  Server_Client(channel, log)
 {
  connect();
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Server_Blob_Client::read_blob_data(Blob blob)
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel_Lock lock(channel);

  buffer.data[0] = 'b';
  buffer.index = 1;
  buffer.write<int64_t>(blob.get_position());
  lock.write(buffer.data, buffer.index);

  lock.read(buffer.data, 9);
  buffer.index = 1;
  const int64_t size = buffer.read<int64_t>();

  Memory_File file;
  joedb::Async_Writer writer(file, 0);
  download(writer, lock, size);

  return file.move_data();
 }
}
