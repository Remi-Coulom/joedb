#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Server_Client.h"

namespace joedb
{
 /// @ingroup concurrency
 class Server_Connection: public Server_Client, public Connection
 {
  protected:
   bool check_matching_content
   (
    const Readonly_Journal &client_journal,
    int64_t server_checkpoint
   );

  public:
   Server_Connection(Channel &channel): Server_Client(channel) {}

   size_t pread(char *data, size_t size, int64_t offset) const;

   int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   ) override;

   int64_t pull
   (
    bool lock_before,
    bool write_data,
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   ) override;

   int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t from,
    int64_t until,
    bool unlock_after
   ) override;

   void unlock() override;
 };
}

#endif
