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
 class Server_Handshake
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int64_t session_id;

  protected:
   Channel &channel;
   std::ostream *log;

  public:
   Server_Handshake(Channel &channel, std::ostream *log);

   int64_t get_session_id() const {return session_id;}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Server_Connection:
 ////////////////////////////////////////////////////////////////////////////
  public Connection,
  public Server_Handshake,
  public Posthumous_Thrower
 {
  private:
   enum {buffer_size = (1 << 13)};
   std::vector<char> buffer;

   std::condition_variable condition;
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;
   enum {keep_alive_interval = 240};

   void lock() override;

   void unlock() override;

   int64_t pull(Writable_Journal &client_journal, char pull_type);

   int64_t pull(Writable_Journal &client_journal) override;

   int64_t lock_pull(Writable_Journal &client_journal) override;

   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override;

   bool check_hash(Readonly_Journal &client_journal) override;

   void keep_alive();

  public:
   Server_Connection(Channel &channel, std::ostream *log);
   ~Server_Connection() override;
 };
}

#endif
