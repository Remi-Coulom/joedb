#include "joedb/concurrency/Server_Client.h"
#include "joedb/Exception.h"

#include <iostream>

#define LOG(x) do {if (log) *log << x;} while (false)

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Server_Client::ping(Channel_Lock &lock)
 ////////////////////////////////////////////////////////////////////////////
 {
  buffer.data[0] = 'i';
  lock.write(buffer.data, 1);
  lock.read(buffer.data, 1);
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
    condition.wait_for
    (
     lock,
     std::chrono::seconds(keep_alive_interval_seconds)
    );

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
 int64_t Server_Client::connect()
 ////////////////////////////////////////////////////////////////////////////
 {
  LOG("Connecting... ");

  buffer.index = 0;
  buffer.write<char>('j');
  buffer.write<char>('o');
  buffer.write<char>('e');
  buffer.write<char>('d');
  buffer.write<char>('b');
  buffer.write<int64_t>(client_version);

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

  if (server_version < 13)
   throw Exception("Unsupported server version");

  session_id = buffer.read<int64_t>();
  const int64_t server_checkpoint = buffer.read<int64_t>();
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

  if (keep_alive_interval_seconds > 0)
  {
   keep_alive_thread_must_stop = false;
   keep_alive_thread = std::thread([this](){keep_alive();});
  }

  return server_checkpoint;
 }

 ////////////////////////////////////////////////////////////////////////////
 Server_Client::Server_Client
 ////////////////////////////////////////////////////////////////////////////
 (
  Channel &channel,
  std::ostream *log,
  int keep_alive_interval_seconds
 ):
  channel(channel),
  log(log),
  keep_alive_interval_seconds(keep_alive_interval_seconds),
  session_id(-1),
  pullonly_server(false)
 {
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
