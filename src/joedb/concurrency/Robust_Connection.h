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

   const Readonly_Journal *saved_journal = nullptr;
   bool saved_content_check = true;

  protected:
   mutable std::unique_ptr<Server_Connection> connection;

   void reconnect(const std::exception *e) const
   {
    if (log && e)
    {
     *log << "Robust_Connection: reconnecting after error: ";
     *log << e->what() << '\n';
    }

    for (int ms = 1000; ; ms = (31 * ms + 32000) >> 5)
    {
     try
     {
      connection.reset();
      channel.reset();
      channel = connector.new_channel();
      connection = std::make_unique<Server_Connection>(*channel);
      if (saved_journal)
       connection->handshake(*saved_journal, saved_content_check);
      return;
     }
     catch (std::exception &e)
     {
      if (log)
      {
       *log << "Robust_Connection: " << ms;
       *log << "ms of sleep after error: " << e.what() << '\n';
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(ms));
     }
    }
   }

#define ROBUSTLY_DO(X)\
   while (true) {try {X;} catch (const std::exception &e) {reconnect(&e);}}

   int64_t pull
   (
    Writable_Journal *client_journal,
    std::chrono::milliseconds wait,
    char pull_type
   )
   {
    ROBUSTLY_DO
    (
     return connection->pull(client_journal, wait, pull_type)
    );
   }

  public:
   Robust_Connection(const Connector &connector, std::ostream *log):
    connector(connector),
    log(log)
   {
    reconnect(nullptr);
   }

   size_t pread(char *data, size_t size, int64_t offset) const
   {
    ROBUSTLY_DO
    (
     return connection->pread(data, size, offset);
    );
   }

   int64_t handshake
   (
    const Readonly_Journal &client_journal,
    bool content_check
   ) override
   {
    saved_journal = &client_journal;
    saved_content_check = content_check;
    ROBUSTLY_DO
    (
     return connection->handshake(client_journal, content_check)
    );
   }

   int64_t pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override
   {
    return pull(&client_journal, wait, 'P');
   }

   int64_t lock_pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override
   {
    return pull(&client_journal, wait, 'L');
   }

   int64_t get_checkpoint
   (
    const Readonly_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override
   {
    return pull(nullptr, wait, 'i');
   }

   int64_t push_until
   (
    const Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    int64_t until_checkpoint,
    bool unlock_after
   ) override
   {
    ROBUSTLY_DO
    (
     return connection->push_until
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
