#ifndef joedb_SSH_Connection_declared
#define joedb_SSH_Connection_declared

#include "joedb/server/Connection.h"
#include "joedb/server/ssh_wrappers.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SSH_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const std::string remote_file_name;
   const bool trace;

   const std::string mutex_file_name;
   const std::string full_remote_name;

   ssh::Session session;
   ssh::SFTP sftp;

   int64_t server_position;

   void lock() override;
   void unlock() override;
   void pull(Journal_File &client_journal) override;
   void push(Readonly_Journal &client_journal) override;

  public:
   SSH_Connection
   (
    std::string user,
    std::string host,
    int port,
    std::string remote_file_name,
    bool trace
   );
 };
}

#endif
