#include "joedb/concurrency/Server_Client.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/error/Exception.h"
#include "joedb/journal/Header.h"
#include "joedb/ui/Progress_Bar.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::log(const std::string &message) noexcept
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   logger.log(std::to_string(get_session_id()) + ": " + message);
  }
  catch (...)
  {
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::ping(Lock<Channel&> &lock)
 ////////////////////////////////////////////////////////////////////////////
 {
  buffer.index = 0;
  buffer.write<char>('D');
  buffer.write<int64_t>(0);
  buffer.write<int64_t>(0);
  lock->write(buffer.data, buffer.index);
  lock->read(buffer.data, 9);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::connect()
 ////////////////////////////////////////////////////////////////////////////
 {
  logger.log("joedb::Server_Client::connect");

  buffer.index = 0;
  buffer.write<std::array<char, 5>>(Header::joedb);
  buffer.write<int64_t>(protocol_version);

  {
   Lock<Channel&> lock(channel);
   lock->write(buffer.data, buffer.index);
   lock->read(buffer.data, 5 + 8 + 8 + 8 + 1);
  }

  buffer.index = 0;

  if (buffer.read<std::array<char, 5>>() != Header::joedb)
   throw Exception("Did not receive \"joedb\" from server");

  const int64_t server_version = buffer.read<int64_t>();

  if (server_version == 0)
   throw Exception("Client version rejected by server");

  logger.log("server_version = " + std::to_string(server_version));

  if (server_version < protocol_version)
   throw Exception("Unsupported server version");

  session_id = buffer.read<int64_t>();
  server_checkpoint = buffer.read<int64_t>();
  const char mode = buffer.read<char>();

  if (mode == 'R')
   pullonly_server = true;
  else if (mode == 'W')
   pullonly_server = false;
  else
   throw Exception("Unexpected server mode");

  log
  (
   "server_checkpoint = " + std::to_string(server_checkpoint) +
   "; mode = " + mode
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::download
 ////////////////////////////////////////////////////////////////////////////
 (
  Async_Writer &writer,
  Lock<Channel&> &lock,
  int64_t size
 ) const
 {
  Progress_Bar progress_bar
  (
   size,
   *const_cast<Logger *>(static_cast<const Logger *>(this))
  );

  for (int64_t read = 0; read < size;)
  {
   const int64_t remaining = size - read;
   const size_t read_size = size_t(std::min(int64_t(buffer.size), remaining));
   const size_t n = lock->read_some(buffer.data, read_size);
   writer.write(buffer.data, n);
   read += int64_t(n);
   progress_bar.print(read);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Client::Server_Client
 ////////////////////////////////////////////////////////////////////////////
 (
  Channel &channel,
  Logger &logger,
  std::chrono::milliseconds keep_alive_interval
 ):
  keep_alive(*this, keep_alive_interval),
  channel(channel),
  logger(logger),
  session_id(-1),
  pullonly_server(false)
 {
  connect();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::disconnect()
 ////////////////////////////////////////////////////////////////////////////
 {
  {
   Lock<Channel&> lock(channel);
   buffer.data[0] = 'Q';
   lock->write(buffer.data, 1);
  }

  keep_alive.stop();
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Client::~Server_Client()
 ////////////////////////////////////////////////////////////////////////////
 {
  try {disconnect();} catch (...) {}
 }
}
