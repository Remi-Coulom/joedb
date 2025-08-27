#ifndef joedb_Robust_Connection_declared
#define joedb_Robust_Connection_declared

#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Connector.h"
#include "joedb/error/Disconnection.h"

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

   const Readonly_Journal *handshake_journal = nullptr;
   Content_Check handshake_content_check = Content_Check::fast;

   static constexpr auto period = std::chrono::seconds(5);
   mutable time_point last_connection_time = clock::now() - period;

   void log_exception(const std::exception *e) const;

  protected:
   mutable std::unique_ptr<Server_Connection> connection;

   void reconnect(const std::exception *e) const;

   template<typename F> auto try_until_success(const F &f) const
   {
    while (true)
    {
     try
     {
      return f();
     }
     catch (const Disconnection &e)
     {
      throw;
     }
     catch (std::exception &e)
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
    reconnect(nullptr);
   }

   size_t pread(char *data, size_t size, int64_t offset) const;

   int64_t handshake
   (
    const Readonly_Journal &client_journal,
    Content_Check content_check
   ) override;

   int64_t pull
   (
    Lock_Action lock_action,
    Data_Transfer data_transfer,
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override;

   int64_t push
   (
    const Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    int64_t until_checkpoint,
    Unlock_Action unlock_action
   ) override;

   void unlock() override;
   bool is_pullonly() const override {return connection->is_pullonly();}
 };
}

#endif
