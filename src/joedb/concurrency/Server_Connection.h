#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Channel.h"
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
   Channel &channel;
   std::ostream *log;

   enum {buffer_size = (1 << 13)};
   std::vector<char> buffer;

   int64_t session_id;
   std::condition_variable condition;
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;
   enum {keep_alive_interval = 240};

   int64_t handshake() final override;

   void lock() final override;

   void unlock() final override;

   int64_t pull(Writable_Journal &client_journal, char pull_type);

   int64_t pull(Writable_Journal &client_journal) final override;

   int64_t lock_pull(Writable_Journal &client_journal) final override;

   void push
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    bool unlock_after
   ) final override;

   bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t checkpoint
   ) final override;

   void keep_alive();

  public:
   Server_Connection(Channel &channel, std::ostream *log);

   int64_t get_session_id() const {return session_id;}

   ~Server_Connection() override;
 };
}

#endif
