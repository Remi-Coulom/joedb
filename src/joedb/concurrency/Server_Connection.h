#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Channel.h"

#include <mutex>
#include <condition_variable>
#include <thread>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 struct Server_Handshake
 ////////////////////////////////////////////////////////////////////////////
 {
  Channel &channel;

  Server_Handshake(Channel &channel);
 };

 ////////////////////////////////////////////////////////////////////////////
 class Server_Connection: public Connection, private Server_Handshake
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   enum {buffer_size = (1 << 13)};
   char *buffer;

   std::condition_variable condition;
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;
   enum {keep_alive_interval = 240};

   int64_t pull(Writable_Journal &client_journal, char pull_type);

   int64_t pull(Writable_Journal &client_journal) override;

   int64_t lock_pull(Writable_Journal &client_journal) override;

   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override;

   void lock() override;

   void unlock() override;

   void keep_alive();

  public:
   Server_Connection(Channel &channel);
   ~Server_Connection() override;
 };
}

#endif
