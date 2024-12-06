#include "joedb/concurrency/Server_Connection.h"

namespace joedb
{
 class Channel_Builder
 {
  public:
   virtual std::unique_ptr<Channel> build() = 0;
   virtual ~Channel_Builder() = default;
 };

 class Robust_Server_Connection: public Connection
 {
  private:
   Channel_Builder &builder;
   std::ostream *log;
   std::unique_ptr<Channel> channel;
   std::unique_ptr<Server_Connection> connection;

   int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   ) override;
   void unlock(Readonly_Journal &client_journal) override;
   int64_t lock_pull(Writable_Journal &client_journal) override;
   int64_t push_until
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    int64_t until_position,
    bool unlock_after
   ) override;

   Connection *get_push_connection() override;

  public:
   Robust_Server_Connection(Channel_Builder &builder, std::ostream *log);
 };
}
