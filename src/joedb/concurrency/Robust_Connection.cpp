#include "joedb/concurrency/Robust_Connection.h"
#include "joedb/ui/interruptible_sleep.h"

namespace joedb
{
 void Robust_Connection::log_exception(const std::exception *e) const
 {
  if (e)
   logger.log("Robust_Connection: exception: " + std::string(e->what()));
 }

 void Robust_Connection::reconnect(const std::exception *e) const
 {
  log_exception(e);

  const int64_t old_server_checkpoint =
   connection ? connection->get_server_checkpoint() : Header::ssize;

  while (true)
  {
   const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
   (
    last_connection_time + period - clock::now()
   );
   if (duration.count() >= 0 && !interruptible_sleep(duration))
    throw Disconnection("Robust_Connection: reconnect interrupted");
   last_connection_time = clock::now();

   try
   {
    auto new_channel = connector.new_channel();
    auto new_connection = std::make_unique<Server_Connection>
    (
     *new_channel,
     logger,
     connector.get_keep_alive_interval()
    );

    if (new_connection->get_server_checkpoint() < old_server_checkpoint)
     throw Disconnection("Robust_Connection: server checkpoint rolled back");

    if (handshake_journal)
     new_connection->handshake(*handshake_journal, handshake_content_check);

    connection = std::move(new_connection);
    channel = std::move(new_channel);

    return;
   }
   catch (const Disconnection &)
   {
    throw;
   }
   catch (std::exception &reconnect_exception)
   {
    log_exception(&reconnect_exception);
   }
  }
 }

 size_t Robust_Connection::pread(char *data, size_t size, int64_t offset) const
 {
  return try_until_success([&]()
  {
   return connection->pread(data, size, offset);}
  );
 }

 int64_t Robust_Connection::handshake
 (
  const Readonly_Journal &client_journal,
  Content_Check content_check
 )
 {
  if (handshake_journal)
   throw Disconnection("Robust_Connection: handshake already performed");

  const int64_t result = try_until_success([&]()
  {
   return connection->handshake(client_journal, content_check);
  });

  handshake_journal = &client_journal;
  handshake_content_check = content_check;

  return result;
 }

 void Robust_Connection::check_matching_content
 (
  const Readonly_Journal &client_journal,
  Content_Check content_check,
  int64_t from,
  int64_t until
 )
 {
  try_until_success([&]()
  {
   connection->check_matching_content(client_journal, content_check, from, until);
  });
 }

 int64_t Robust_Connection::pull
 (
  Lock_Action lock_action,
  Data_Transfer data_transfer,
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  return try_until_success([&]()
  {
   return connection->pull(lock_action, data_transfer, client_journal, wait);
  });
 }

 int64_t Robust_Connection::push
 (
  const Readonly_Journal &client_journal,
  int64_t from_checkpoint,
  int64_t until_checkpoint,
  Unlock_Action unlock_action
 )
 {
  return try_until_success([&]()
  {
   return connection->push
   (
    client_journal,
    from_checkpoint,
    until_checkpoint,
    unlock_action
   );
  });
 }

 void Robust_Connection::ping()
 {
  try_until_success([&](){connection->ping();});
 }

 void Robust_Connection::unlock()
 {
  try
  {
   connection->unlock();
  }
  catch (const std::exception &e)
  {
   logger.log("Robust_Connection::unlock() error: " + std::string(e.what()));
  }
 }
}
