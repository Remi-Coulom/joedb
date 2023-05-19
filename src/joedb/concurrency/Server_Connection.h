#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Thread_Safe_Channel.h"
#include "joedb/Posthumous_Thrower.h"

#include <mutex>
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

   enum {buffer_size = (1 << 13)};
   std::vector<char> buffer;

   int64_t session_id;
   std::condition_variable condition;
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;
   enum {keep_alive_interval = 240};

   int64_t handshake() final;
   void lock() final;
   void unlock() final;
   int64_t pull(char pull_type);
   int64_t shared_pull(char pull_type);
   int64_t pull() final;
   int64_t lock_pull() final;
   void push (int64_t server_position, bool unlock_after) final;
   bool check_matching_content(int64_t server_checkpoint);
   void keep_alive();

  public:
   Server_Connection
   (
    Writable_Journal &client_journal,
    Channel &channel,
    std::ostream *log
   );

   int64_t get_session_id() const {return session_id;}
   Thread_Safe_Channel &get_channel() {return channel;}

   ~Server_Connection() override;
 };
}

#endif
