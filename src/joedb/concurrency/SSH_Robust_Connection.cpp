#ifdef JOEDB_HAS_SSH

#include "joedb/concurrency/SSH_Robust_Connection.h"
#include "joedb/Exception.h"

#include <thread>
#include <chrono>
#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int64_t SSH_Robust_Connection::retry
 ////////////////////////////////////////////////////////////////////////////
 (
  std::function<int64_t()> f
 )
 {
  while (true)
  {
   if (connection)
    try
    {
     return f();
    }
    catch(const std::runtime_error &e)
    {
     if (trace)
      std::cerr << "Error: " << e.what() << '\n';
    }

   reconnect();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t SSH_Robust_Connection::pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return retry
  (
   [&client_journal,this](){return connection->pull(client_journal);}
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t SSH_Robust_Connection::lock_pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return retry
  (
   [&client_journal,this](){return connection->lock_pull(client_journal);}
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Robust_Connection::push_unlock
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_position
 )
 {
  retry
  (
   [&client_journal,server_position,this]()->int64_t
   {
    connection->push_unlock(client_journal, server_position);
    return 0;
   }
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Robust_Connection::reset()
 ////////////////////////////////////////////////////////////////////////////
 {
  connection.reset
  (
   new SSH_Connection
   (
    user,
    host,
    port,
    remote_file_name,
    trace,
    ssh_log_level
   )
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 SSH_Robust_Connection::SSH_Robust_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  std::string user,
  std::string host,
  int port,
  std::string remote_file_name,
  bool trace,
  int ssh_log_level
 ):
  user(user),
  host(host),
  port(port),
  remote_file_name(remote_file_name),
  trace(trace),
  ssh_log_level(ssh_log_level),
  sleep_time(10)
 {
  retry([]()->int64_t{return 0;});
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Robust_Connection::reconnect()
 ////////////////////////////////////////////////////////////////////////////
 {
  for (bool first_attempt = true; ; first_attempt = false)
  {
   if (connection || !first_attempt)
   {
    if (trace)
     std::cerr << "Sleeping for " << sleep_time << " seconds...\n";

    std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
   }

   if (trace)
    std::cerr << "Connecting... ";
   try
   {
    reset();
    if (trace)
     std::cerr << "Success!\n";
    break;
   }
   catch(const std::runtime_error &e)
   {
    if (trace)
     std::cerr << "Error: " << e.what() << '\n';
   }
  }
 }
}

#endif
