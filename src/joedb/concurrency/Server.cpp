#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/protocol_version.h"
#include "joedb/journal/File_Hasher.h"
#include "joedb/ui/Progress_Bar.h"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/compose.hpp>

#include <cstdio>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 const std::map<char, const char *> Server::request_description
 ////////////////////////////////////////////////////////////////////////////
 {
  {'H', "get fast SHA-256 hash code"},
  {'I', "get full SHA-256 hash code"},
  {'r', "read a range of bytes"},
  {'D', "get checkpoint"},
  {'E', "lock"},
  {'F', "pull"},
  {'G', "lock_pull"},
  {'L', "lock"},
  {'M', "unlock"},
  {'N', "lock_push"},
  {'O', "push_unlock"}
 };

 ////////////////////////////////////////////////////////////////////////////
 Server::Session::Session
 ////////////////////////////////////////////////////////////////////////////
 (
  Server &server,
  boost::asio::local::stream_protocol::socket &&socket
 ):
  joedb::asio::Server::Session(server, std::move(socket)),
  channel(server.thread_pool)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::Session::cleanup()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (locking)
  {
   if (get_server().log_level > 2)
    log("removing lock held by dying session.");
   unlock();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::lock(Session &session)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!locked)
  {
   if (log_level > 2)
    session.log("obtained lock immediately");
  }
  else
  {
   if (log_level > 2)
    session.log("waiting for lock");

   lock_waiters.emplace(&session);
   co_await session.channel.async_receive(boost::asio::use_awaitable);

   if (log_level > 2)
    session.log("obtained lock after waiting");

   lock_waiters.pop();
  }

  locked = true;
  session.locking = true;

  if (!client_lock)
  {
   if (!writable_journal_client)
   {
    session.log("error: locking pull-only server");
    session.buffer.data[0] = 'R';
   }
   else
    client_lock.emplace(*writable_journal_client);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Server::Session::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (get_server().log_level > 2)
   log("unlock");

  locking = false;

  if (get_server().lock_waiters.empty())
   get_server().locked = false;
  else
   get_server().lock_waiters.front()->channel.try_send(boost::system::error_code{});
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<size_t> Server::Session::read_buffer
 ///////////////////////////////////////////////////////////////////////////
 (
  const size_t offset,
  const size_t size
 )
 {
  const size_t result = co_await boost::asio::async_read
  (
   socket,
   boost::asio::buffer(buffer.data + offset, size),
   boost::asio::use_awaitable
  );

  buffer.index = offset;

  co_return result;
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::write_buffer()
 ///////////////////////////////////////////////////////////////////////////
 {
  co_await boost::asio::async_write
  (
   socket,
   boost::asio::buffer(buffer.data, buffer.index),
   boost::asio::use_awaitable
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::send(Async_Reader reader)
 ///////////////////////////////////////////////////////////////////////////
 {
  log
  (
   "sending to client, from = " + std::to_string(reader.get_current()) +
   ", until = " + std::to_string(reader.get_end())
  );

  Progress_Bar progress_bar(reader.get_remaining(), nullptr);

  buffer.index = 1;
  buffer.write<int64_t>(reader.get_end());

  while (buffer.index + reader.get_remaining() > 0)
  {
   buffer.index += reader.read
   (
    buffer.data + buffer.index,
    buffer.size - buffer.index
   );

   if (reader.is_end_of_file())
    throw Exception("unexpected end of file");

   // refresh_lock_timeout(session);
   co_await write_buffer();
   buffer.index = 0;

   progress_bar.print_remaining(reader.get_remaining());
  }
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::check_hash()
 ///////////////////////////////////////////////////////////////////////////
 {
  co_await read_buffer(1, 40);

  const auto checkpoint = buffer.read<int64_t>();
  const auto hash = buffer.read<SHA_256::Hash>();

  const Readonly_Journal &journal = get_server().client.get_journal();

  if
  (
   checkpoint > journal.get_checkpoint() ||
   hash != (buffer.data[0] == 'H' // ??? takes_time
   ? Journal_Hasher::get_fast_hash(journal, checkpoint)
   : Journal_Hasher::get_full_hash(journal, checkpoint))
  )
  {
   buffer.data[0] = 'h';
  }

  log
  (
   "hash for checkpoint = " + std::to_string(checkpoint) +
   ", result = " + buffer.data[0]
  );

  buffer.index = 1;
  co_await write_buffer();
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::handshake()
 ///////////////////////////////////////////////////////////////////////////
 {
  co_await read_buffer(0, 13);

  if (buffer.read<std::array<char, 5>>() != Header::joedb)
   throw Exception("handshake does not start by joedb");

  const int64_t client_version = buffer.read<int64_t>();
  if (get_server().log_level > 0)
   log("client_version = " + std::to_string(client_version));

  const int64_t version =
   client_version < protocol_version ? 0 : protocol_version;

  const bool writable =
   get_server().writable_journal_client &&
   !get_server().client.is_pullonly();

  buffer.index = 5;
  buffer.write<int64_t>(version);
  buffer.write<int64_t>(id);
  buffer.write<int64_t>(get_server().client.get_journal_checkpoint());
  buffer.write<char>(writable ? 'W' : 'R');

  co_await write_buffer();
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::read()
 ///////////////////////////////////////////////////////////////////////////
 {
  co_await read_buffer(1, 16);

  int64_t from = buffer.read<int64_t>();
  int64_t until = buffer.read<int64_t>();

  if (until > get_server().client.get_journal_checkpoint())
   until = get_server().client.get_journal_checkpoint();
  if (from > until)
   from = until;

  co_await send
  (
   get_server().client.get_journal().get_async_reader(from, until)
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::pull
 ///////////////////////////////////////////////////////////////////////////
 (
  bool lock_before,
  bool send_data
 )
 {
  co_await read_buffer(1, 16);

  const std::chrono::milliseconds wait{buffer.read<int64_t>()};
  int64_t pull_checkpoint = buffer.read<int64_t>();

  if (!get_server().client_lock)
   get_server().client.pull(); // ??? takes_time

  if
  (
   wait.count() > 0 &&
   pull_checkpoint == get_server().client.get_journal_checkpoint()
  )
  {
   if (get_server().log_level > 2)
   {
    log
    (
     "waiting at checkpoint = " + std::to_string(pull_checkpoint) +
     " for " + std::to_string(double(wait.count()) * 0.001) + 's'
    );
   }

   get_server().pull_waiters.emplace(this);
   boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
   timer.expires_after(wait);
   co_await timer.async_wait(boost::asio::use_awaitable);
   // TODO use channel to trigger early interruption
  }

  if (lock_before)
   co_await get_server().lock(*this);

  if (!send_data)
   pull_checkpoint = get_server().client.get_journal_checkpoint();

  co_await send
  (
   get_server().client.get_journal().get_async_tail_reader(pull_checkpoint)
  );
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::push
 ///////////////////////////////////////////////////////////////////////////
 (
  bool unlock_after
 )
 {
  co_await read_buffer(1, 16);

  const int64_t from = buffer.read<int64_t>();
  const int64_t until = buffer.read<int64_t>();

  if (get_server().locked && !locking)
   throw Exception("trying to push while someone else is locking");
  else if (!get_server().locked)
  {
   if (get_server().log_level > 2)
    log("taking the lock for push attempt.");
   co_await get_server().lock(*this);
  }

  const bool conflict =
  (
   !locking ||
   from != get_server().client.get_journal().get_checkpoint()
  );

  log
  (
   "receiving from client, from = " + std::to_string(from) +
   ", until = " + std::to_string(until)
  );

  char status = buffer.data[0];
  std::optional<Async_Writer> writer;

  if (!get_server().writable_journal_client)
   status = 'R';
  else if (conflict)
   status = 'C';
  else
  {
   if (get_server().client_lock)
   {
    writer.emplace
    (
     get_server().client_lock->get_journal().get_async_tail_writer()
    );
   }
  }

  if (get_server().log_level > 2)
  {
   log
   (
    "receiving from client, from = " + std::to_string(from) +
    ", until = " + std::to_string(until)
   );
  }

  int64_t remaining_size = until - from;

  while (remaining_size > 0)
  {
   const size_t size = co_await read_buffer
   (
    0,
    size_t(std::min(remaining_size, int64_t(buffer.size)))
   );

   if (writer)
    writer->write(buffer.data, size); // ??? takes_time

   remaining_size -= int64_t(size);
   if (get_server().log_level > 3 && remaining_size > 0)
    log("remaining_size = " + std::to_string(remaining_size));
  }

  if (until > from)
  { 
   if (writer)
   {
    if (get_server().client_lock)
    {
     get_server().client_lock->get_journal().soft_checkpoint_at
     (
      writer->get_position()
     );

     if (get_server().client.is_shared() && unlock_after)
     {
      get_server().client_lock->checkpoint_and_push_unlock();
      get_server().client_lock.reset();
     }
     else
      get_server().client_lock->checkpoint_and_push();
    }
   }
  }

  if (get_server().log_level > 2)
   log(std::string("done pushing, status = ") + status);

  if (unlock_after)
   unlock();

  auto &waiters = get_server().pull_waiters;
  while (!waiters.empty())
  {
   waiters.front()->channel.try_send(boost::system::error_code{});
   waiters.pop();
  }

  buffer.data[0] = status;
  buffer.index = 1;
  co_await write_buffer();
 }

 ///////////////////////////////////////////////////////////////////////////
 boost::asio::awaitable<void> Server::Session::run()
 ///////////////////////////////////////////////////////////////////////////
 {
  co_await handshake();

  while (true)
  {
   co_await read_buffer(0, 1);

   const char code = buffer.data[0];

   if (server.get_log_level() > 2)
   {
    const auto i = request_description.find(code);
    if (i != request_description.end())
     log(std::string("received request ") + code + ": " + i->second);
    else
     log(std::string("unknown code: ") + code);
   }

   switch (code)
   {
    case 'H': case 'I':
     co_await check_hash();
    break;

    case 'r':
     co_await read();
    break;

    case 'D': case 'E': case 'F': case 'G':
     co_await pull(code & 1, code & 2);
    break;

    case 'M':
     unlock();
    break;

    case 'N': case 'O':
     co_await push(code == 'O');
    break;

    default:
     co_return;
    break;
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Server::Server
 ////////////////////////////////////////////////////////////////////////////
 (
  Logger &logger,
  int log_level,
  int thread_count,
  std::string endpoint_path,
  Client &client,
  std::chrono::milliseconds lock_timeout
 ):
  joedb::asio::Server(logger, log_level, thread_count, std::move(endpoint_path)),
  client(client),
  writable_journal_client(dynamic_cast<Writable_Journal_Client*>(&client)),
  lock_timeout(lock_timeout),
  lock_timeout_timer(thread_pool),
  locked(false)
 {
  if (writable_journal_client)
   writable_journal_client->push_if_ahead();
 }
}

#undef LOGID
#undef LOG
