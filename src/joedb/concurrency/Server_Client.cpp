#include "joedb/concurrency/Server_Client.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/Exception.h"
#include "joedb/io/Progress_Bar.h"

#include <iostream>
#include <optional>

#define LOG(x) do {if (log) *log << x;} while (false)

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::ping(Channel_Lock &lock)
 ////////////////////////////////////////////////////////////////////////////
 {
  buffer.index = 0;
  buffer.write<char>('i');
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
  buffer.write<char>('j');
  buffer.write<char>('o');
  buffer.write<char>('e');
  buffer.write<char>('d');
  buffer.write<char>('b');
  buffer.write<int64_t>(protocol_version);

  {
   Channel_Lock lock(channel);
   lock.write(buffer.data, buffer.index);
   LOG("Waiting for \"joedb\"... ");
   lock.read(buffer.data, 5 + 8 + 8 + 8 + 1);
  }

  buffer.index = 0;

  if
  (
   buffer.read<char>() != 'j' ||
   buffer.read<char>() != 'o' ||
   buffer.read<char>() != 'e' ||
   buffer.read<char>() != 'd' ||
   buffer.read<char>() != 'b'
  )
  {
   throw Exception("Did not receive \"joedb\" from server");
  }

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
 )
 {
  LOG("downloading, size = " << size);

  std::optional<io::Progress_Bar> progress_bar;
  if (size > buffer.ssize && log)
   progress_bar.emplace(size, *log);

  for (int64_t read = 0; read < size;)
  {
   const int64_t remaining = size - read;
   const size_t read_size = size_t
   (
    std::min(int64_t(buffer.size), remaining)
   );
   const size_t n = lock.read_some(buffer.data, read_size);
   writer.write(buffer.data, n);
   read += int64_t(n);
   if (progress_bar)
    progress_bar->print(read);
  }

  if (!progress_bar)
   LOG(" OK\n");
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Client::Server_Client(Channel &channel):
 ////////////////////////////////////////////////////////////////////////////
  keep_alive_interval(std::chrono::seconds{240}),
  channel(channel),
  log(nullptr),
  session_id(-1),
  pullonly_server(false)
 {
  connect();
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Client::~Server_Client()
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   Channel_Lock lock(channel);
   keep_alive_thread_must_stop = true;
   buffer.data[0] = 'Q';
   lock.write(buffer.data, 1);
  }
  catch (...)
  {
   postpone_exception("Could not write to server");
  }

  try
  {
   condition.notify_one();
   if (keep_alive_thread.joinable())
    keep_alive_thread.join();
  }
  catch (...)
  {
   postpone_exception("Could not join keep-alive thread");
  }
 }
}

#undef LOG
