#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Mutex.h"
#include "joedb/concurrency/net.h"

#include <mutex>
#include <condition_variable>
#include <thread>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_Connection: public Connection, public Mutex
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   net::io_context io_context;
   net::ip::tcp::socket socket;

   std::mutex mutex;
   std::condition_variable condition;
   bool keep_alive_thread_must_stop;
   std::thread keep_alive_thread;
   enum {keep_alive_interval = 240};

   enum {buffer_size = (1 << 13)};
   char *buffer;

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
   Server_Connection(const char *host_name, const char *port_name);
   ~Server_Connection() override;
 };
}

#endif
