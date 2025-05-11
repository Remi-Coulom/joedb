#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Server_Client.h"

namespace joedb
{
 /// @ingroup concurrency
 class Server_Connection: public Server_Client, public Connection
 {
  public:
   Server_Connection(Channel &channel, std::ostream *log = nullptr):
    Server_Client(channel, log)
   {
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
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   ) override;

   int64_t push
   (
    const Readonly_Journal &client_journal,
    int64_t from,
    int64_t until,
    Unlock_Action unlock_action
   ) override;

   void unlock() override;

   bool is_pullonly() const override
   {
    return this->pullonly_server;
   }
 };
}

#endif
