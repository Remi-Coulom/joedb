#include "joedb/concurrency/Server_Client.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/error/Exception.h"
#include "joedb/ui/Progress_Bar.h"
#include "joedb/journal/Header.h"

#include <iostream>

#define LOG(x) do {if (log) *log << x;} while (false)

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::ping(Channel_Lock &lock)
 ////////////////////////////////////////////////////////////////////////////
 {
  buffer.index = 0;
  buffer.write<char>('D');
  buffer.write<int64_t>(0);
  buffer.write<int64_t>(0);
  lock.write(buffer.data, buffer.index);
  lock.read(buffer.data, 9);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::ping()
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel_Lock lock(channel);
  ping(lock);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::keep_alive()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   Channel_Lock lock(channel);

   while (!keep_alive_thread_must_stop)
   {
    condition.wait_for(lock, keep_alive_interval);

    if (keep_alive_thread_must_stop)
     break;

    ping(lock);
   }
  }
  catch(...)
  {
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::connect()
 ////////////////////////////////////////////////////////////////////////////
 {
  LOG("Connecting... ");

  buffer.index = 0;
  buffer.write<std::array<char, 5>>(Header::joedb);
  buffer.write<int64_t>(protocol_version);

  {
   Channel_Lock lock(channel);
   lock.write(buffer.data, buffer.index);
   lock.read(buffer.data, 5 + 8 + 8 + 8 + 1);
  }

  buffer.index = 0;

  if (buffer.read<std::array<char, 5>>() != Header::joedb)
   throw Exception("Did not receive \"joedb\" from server");

  const int64_t server_version = buffer.read<int64_t>();

  if (server_version == 0)
   throw Exception("Client version rejected by server");

  LOG("server_version = " << server_version << ". ");

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

  LOG
  (
   "session_id = " << session_id <<
   "; server_checkpoint = " << server_checkpoint <<
   "; mode = " << mode <<
   ". OK.\n"
  );

  if (keep_alive_interval.count() > 0)
  {
   keep_alive_thread_must_stop = false;
   keep_alive_thread = std::thread([this](){keep_alive();});
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::download
 ////////////////////////////////////////////////////////////////////////////
 (
  Async_Writer &writer,
  Channel_Lock &lock,
  int64_t size
 ) const
 {
  LOG("downloading");
  Progress_Bar progress_bar(size, log);

  for (int64_t read = 0; read < size;)
  {
   const int64_t remaining = size - read;
   const size_t read_size = size_t(std::min(buffer.ssize, remaining));
   const size_t n = lock.read_some(buffer.data, read_size);
   writer.write(buffer.data, n);
   read += int64_t(n);
   progress_bar.print(read);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Client::Server_Client(Channel &channel, std::ostream *log):
 ////////////////////////////////////////////////////////////////////////////
  keep_alive_interval(std::chrono::seconds{240}),
  channel(channel),
  log(log),
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
   Channel_Lock lock(channel);
   keep_alive_thread_must_stop = true;
  }

  condition.notify_one();
  if (keep_alive_thread.joinable())
   keep_alive_thread.join();
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Client::~Server_Client()
 ////////////////////////////////////////////////////////////////////////////
 {
  try { disconnect(); } catch (...) {}
 }
}

#undef LOG
