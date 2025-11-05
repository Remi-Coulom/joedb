#ifndef joedb_Server_Client_declared
#define joedb_Server_Client_declared

#include "joedb/concurrency/Channel.h"
#include "joedb/concurrency/Keep_Alive_Thread.h"
#include "joedb/Thread_Safe.h"
#include "joedb/journal/Buffer.h"
#include "joedb/journal/Async_Writer.h"
#include "joedb/error/Logger.h"

#include <chrono>

namespace joedb
{
 /// @ingroup concurrency
 class Server_Client: public Logger, public Ping_Client
 {
  friend class Server_File;

  private:
   void locked_ping(Lock<Channel&> &lock) override;
   Keep_Alive_Thread keep_alive;

   void connect();
   void disconnect();

  protected:
   mutable Thread_Safe<Channel&> channel;
   Logger &logger;
   bool connected;

   mutable Buffer<13> buffer;

   int64_t session_id;
   bool pullonly_server;
   int64_t server_checkpoint;

   void download
   (
    Async_Writer &writer,
    Lock<Channel&> &lock,
    int64_t size
   ) const;

   void log(const std::string &message) noexcept override;

  public:
   ///
   /// @param keep_alive_interval: a background thread will send a ping
   /// to the server at this interval. Setting this parameter to zero disables
   /// the keep-alive background thread.
   ///
   Server_Client
   (
    Channel &channel,
    Logger &logger = Logger::dummy_logger,
    std::chrono::milliseconds keep_alive_interval = std::chrono::milliseconds(0)
   );

   int64_t get_session_id() const {return session_id;}
   Thread_Safe<Channel&> &get_channel() override {return channel;}

   ~Server_Client();
 };
}

#endif
