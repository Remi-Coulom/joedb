#ifndef joedb_Robust_Connection_declared
#define joedb_Robust_Connection_declared

#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Connector.h"

#include <ostream>

namespace joedb
{
 /// @ref Server_Connection that automatically reconnects on error
 ///
 /// @ingroup concurrency
 class Robust_Connection: public Connection
 {
  private:
   const Connector &connector;
   std::ostream *log;
   mutable std::unique_ptr<Channel> channel;

   Readonly_Journal *handshake_journal = nullptr;
   bool handshake_content_check = true;

  protected:
   mutable std::unique_ptr<Server_Connection> connection;

   void reconnect(const std::exception *e) const
   {
    if (log && e)
    {
     *log << "Robust_Connection: reconnecting after error: ";
     *log << e->what() << std::endl;
    }

    for (int ms = 1000; ; ms = (31 * ms + 32000) >> 5)
    {
     std::this_thread::sleep_for(std::chrono::milliseconds(ms));

     try
     {
      connection.reset();
      channel.reset();
      channel = connector.new_channel();
      connection = std::make_unique<Server_Connection>(*channel);
      connection->set_log(log);
      if (handshake_journal)
       connection->handshake(*handshake_journal, handshake_content_check);
      return;
     }
     catch (std::exception &e)
     {
      if (log)
      {
       *log << "Robust_Connection: error while reconnecting: ";
       *log << e.what() << std::endl;
      }
     }
    }
   }

  public:
   Robust_Connection(const Connector &connector, std::ostream *log):
    connector(connector),
    log(log)
   {
    reconnect(nullptr);
   }

#define ROBUSTLY_DO(X)\
   while (true) {try {X;} catch (const std::exception &e) {reconnect(&e);}}

   size_t pread(char *data, size_t size, int64_t offset) const
   {
    ROBUSTLY_DO
    (
     return connection->pread(data, size, offset);
    );
   }

   int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   ) override
   {
    handshake_journal = &client_journal;
    handshake_content_check = content_check;
    ROBUSTLY_DO
    (
     return connection->handshake(client_journal, content_check)
    );
   }

   int64_t pull
   (
    bool lock_before,
    bool write_data,
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override
   {
    ROBUSTLY_DO
    (
     return connection->pull(lock_before, write_data, client_journal, wait)
    );
   }

   int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    int64_t until_checkpoint,
    bool unlock_after
   ) override
   {
    ROBUSTLY_DO
    (
     return connection->push
     (
      client_journal,
      from_checkpoint,
      until_checkpoint,
      unlock_after
     )
    );
   }

   void unlock() override
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
#undef ROBUSTLY_DO
 };
}

#endif
