#ifndef joedb_SSH_Robust_Connection_declared
#define joedb_SSH_Robust_Connection_declared

#include "joedb/concurrency/SSH_Connection.h"

#ifdef JOEDB_HAS_SSH

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SSH_Robust_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const std::string user;
   const std::string host;
   const int port;
   const std::string remote_file_name;
   const bool trace;
   const int ssh_log_level;

   int sleep_time;

   std::unique_ptr<SSH_Connection> connection;

   int64_t retry(std::function<int64_t()> f);

   int64_t pull(Writable_Journal &client_journal) override;
   int64_t lock_pull(Writable_Journal &client_journal) override;
   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override;

   void reset();

  public:
   SSH_Robust_Connection
   (
    std::string user,
    std::string host,
    int port,
    std::string remote_file_name,
    bool trace,
    int ssh_log_level
   );

   void ensure_connected();
   ssh::Session &get_session();
   ssh::SFTP &get_sftp();
 };
}

#else

namespace joedb
{
 typedef SSH_Connection SSH_Robust_Connection;
}

#endif
#endif
