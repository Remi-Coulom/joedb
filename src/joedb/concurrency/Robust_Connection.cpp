#include "joedb/concurrency/Robust_Connection.h"

namespace joedb
{
 void Robust_Connection::log_exception(const std::exception *e) const
 {
  if (e && log)
   *log << "Robust_Connection: " << e->what() << std::endl;
 }

 void Robust_Connection::reconnect(const std::exception *e) const
 {
  log_exception(e);

  while (true)
  {
   std::this_thread::sleep_until(last_connection_time + period);
   last_connection_time = clock::now();

   try
   {
    connection.reset();
    channel.reset();
    channel = connector.new_channel();
    connection = std::make_unique<Server_Connection>(*channel, log);
    if (handshake_journal)
     connection->handshake(*handshake_journal, handshake_content_check);
    return;
   }
   catch (const Disconnection &disconnection_exception)
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
  const int64_t result = try_until_success([&]()
  {
   return connection->handshake(client_journal, content_check);
  });

  handshake_journal = &client_journal;
  handshake_content_check = content_check;

  return result;
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

 void Robust_Connection::unlock()
 {
  try
  {
   connection->unlock();
  }
  catch (const std::exception &e)
  {
   if (log)
    *log << "Robust_Connection::unlock() error: " << e.what() << '\n';
  }
 }
}
