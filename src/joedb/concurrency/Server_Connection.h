#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Server_Client.h"

namespace joedb
{
 /// \ingroup concurrency
 class Server_Connection: public Server_Client, public Connection
 {
  protected:
   int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   ) override;

   int64_t pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override;

   int64_t lock_pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait
   ) override;

   int64_t push_until
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    int64_t until_position,
    bool unlock_after
   ) override;

   void unlock(Readonly_Journal &client_journal) override;

   bool is_pullonly() const override;

   int64_t pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait,
    char pull_type,
    bool has_data
   );

   int64_t shared_pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait,
    char pull_type
   );

   bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t server_checkpoint
   );

  public:
   Server_Connection(Channel &channel): Server_Client(channel) {}
 };
}

#endif
