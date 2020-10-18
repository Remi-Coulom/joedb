#ifdef JOEDB_HAS_SSH

#ifndef joedb_SSH_Connection_declared
#define joedb_SSH_Connection_declared

#include "joedb/server/Connection.h"
#include "joedb/server/ssh_wrappers.h"

#include <memory>
#include <functional>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SSH_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class SSH_Robust_Connection;

  private:
   const std::string remote_file_name;
   const bool trace;

   const std::string mutex_file_name;
   const std::string full_remote_name;

   ssh::Session session;
   ssh::SFTP sftp;

   void lock();
   void unlock();
   int64_t raw_pull(Writable_Journal &client_journal);
   void raw_push(Readonly_Journal &client_journal, int64_t server_position);

   int64_t pull(Writable_Journal &client_journal) override
   {
    lock();
    const int64_t result = raw_pull(client_journal);
    unlock();
    return result;
   }

   int64_t lock_pull(Writable_Journal &client_journal) override
   {
    lock();
    return raw_pull(client_journal);
   }

   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override
   {
    raw_push(client_journal, server_position);
    unlock();
   }

  public:
   SSH_Connection
   (
    std::string user,
    std::string host,
    int port,
    std::string remote_file_name,
    bool trace,
    int ssh_log_level
   );
 };

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

   void set_sleep_time(int seconds) {sleep_time = seconds;}
 };
}

#endif
#endif
