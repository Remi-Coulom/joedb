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
  using clock = std::chrono::steady_clock;
  using time_point = std::chrono::time_point<clock>;

  private:
   const Connector &connector;
   std::ostream *log;
   mutable std::unique_ptr<Channel> channel;

   Readonly_Journal *handshake_journal = nullptr;
   bool handshake_content_check = true;

   static constexpr auto period = std::chrono::seconds(2);
   mutable time_point last_connection_time = clock::now() - period;

   void log_exception(const std::exception *e) const
   {
    if (e && log)
     *log << "Robust_Connection: " << e->what() << std::endl;
   }

  protected:
   mutable std::unique_ptr<Server_Connection> connection;

   void reconnect(const std::exception *e) const
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
      connection = std::make_unique<Server_Connection>(*channel);
      connection->set_log(log);
      if (handshake_journal)
       connection->handshake(*handshake_journal, handshake_content_check);
      return;
     }
     catch (Content_Mismatch &e)
     {
      throw;
     }
     catch (std::exception &e)
     {
      log_exception(&e);
     }
    }
   }

   template<typename F> auto try_until_success(const F &f) const
   {
    while (true)
    {
     try
     {
      return f();
     }
     catch (Content_Mismatch &e)
     {
      throw;
     }
     catch (const std::exception &e)
     {
      reconnect(&e);
     }
    }
   }

  public:
   Robust_Connection(const Connector &connector, std::ostream *log):
    connector(connector),
    log(log)
   {
    last_connection_time = clock::now() - period;
    reconnect(nullptr);
   }

   size_t pread(char *data, size_t size, int64_t offset) const
   {
    return try_until_success([&]()
    {
     return connection->pread(data, size, offset);}
    );
   }

   int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   ) override
   {
    const int64_t result = try_until_success([&]()
    {
     return connection->handshake(client_journal, content_check);
    });

    handshake_journal = &client_journal;
    handshake_content_check = content_check;

    return result;
   }

   int64_t pull
   (
    bool lock_before,
    bool write_data,
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override
   {
    return try_until_success([&]()
    {
     return connection->pull(lock_before, write_data, client_journal, wait);
    });
   }

   int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    int64_t until_checkpoint,
    bool unlock_after
   ) override
   {
    return try_until_success([&]()
    {
     return connection->push
     (
      client_journal,
      from_checkpoint,
      until_checkpoint,
      unlock_after
     );
    });
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
 };
}

#endif
