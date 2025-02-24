#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Thread_Safe_Channel.h"
#include "joedb/Posthumous_Thrower.h"
#include "joedb/journal/Buffer.h"

#include <condition_variable>
#include <thread>
#include <iosfwd>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_Connection: public Connection, public Posthumous_Thrower
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Thread_Safe_Channel channel;
   std::ostream *log;
   const int keep_alive_interval_seconds;

   Buffer<13> buffer;

   int64_t session_id;
   bool pullonly_server;
   std::condition_variable condition;
   void ping(Channel_Lock &lock);
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;

   //
   // Pullonly Connection overrides
   //
   int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   ) override;

   int64_t pull
   (
    Writable_Journal &client_journal,
    int64_t wait_milliseconds
   ) override;

   Connection *get_push_connection() override;

   //
   // Connection overrides
   //
   int64_t lock_pull
   (
    Writable_Journal &client_journal,
    int64_t wait_milliseconds
   ) override;

   int64_t push_until
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    int64_t until_position,
    bool unlock_after
   ) override;

   void unlock(Readonly_Journal &client_journal) override;

   int64_t pull(Writable_Journal &client_journal, int64_t wait_milliseconds, char pull_type);
   int64_t shared_pull(Writable_Journal &client_journal, int64_t wait_milliseconds, char pull_type);

   bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t server_checkpoint
   );

   void keep_alive();

  public:
   Server_Connection
   (
    Channel &channel,
    std::ostream *log,
    int keep_alive_interval_seconds = 240
   );

   static constexpr int64_t client_version = 12;
   int64_t get_session_id() const {return session_id;}
   Thread_Safe_Channel &get_channel() {return channel;}
   void ping();

   ~Server_Connection() override;
 };
}

#endif
