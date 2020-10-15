#ifdef JOEDB_HAS_SSH

#include "joedb/server/SSH_Connection.h"
#include "joedb/Exception.h"

#include <fcntl.h>
#include <thread>
#include <chrono>
#include <iostream>

//
// Windows does not define those
//
#ifndef S_IRUSR
#define	S_IRUSR	0000400
#endif

#ifndef S_IWUSR
#define	S_IWUSR	0000200
#endif

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (trace)
   std::cerr << full_remote_name << ": lock()... ";

  while (true)
  {
   sftp_file file = sftp_open
   (
    sftp.get(),
    mutex_file_name.c_str(),
    O_CREAT | O_EXCL,
    S_IRUSR
   );

   if (file)
   {
    sftp_close(file);
    break;
   }
   else
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (trace)
   std::cerr << "done.\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (trace)
   std::cerr << full_remote_name << ": unlock()\n";

  if (sftp_unlink(sftp.get(), mutex_file_name.c_str()) < 0)
   throw Exception("Error removing remote mutex");
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::raw_pull(Journal_File &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (trace)
   std::cerr << full_remote_name << ": pull()... ";

  server_position = 0;

  try
  {
   ssh::SFTP_Attributes attributes
   (
    sftp_stat(sftp.get(), remote_file_name.c_str())
   );

   server_position = int64_t(attributes.get()->size);
  }
  catch (const joedb::Exception &)
  {
  }

  const int64_t client_position = client_journal.get_checkpoint_position();

  if (client_position < server_position)
  {
   std::vector<char> v(size_t(server_position - client_position));

   if (trace)
    std::cerr << v.size() << " bytes... ";

   sftp_file file = sftp_open
   (
    sftp.get(),
    remote_file_name.c_str(),
    O_RDONLY,
    S_IRUSR | S_IWUSR
   );

   if (file)
   {
    int seek_result = sftp_seek64(file, uint64_t(client_position));
    ssize_t read_result = sftp_read(file, v.data(), v.size());
    sftp_close(file);

    if (seek_result < 0 || read_result != ssize_t(v.size()))
     throw Exception("Error during sftp_read");

    client_journal.append_raw_tail(v);
   }
   else
    throw Exception("Could not open remote file for reading");
  }
  else if (server_position > 0 && client_position > server_position)
   throw Exception("Trying to pull when ahead of server");

  if (trace)
   std::cerr << "done\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::raw_push(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (trace)
   std::cerr << full_remote_name << ": push()... ";

  const int64_t client_position = client_journal.get_checkpoint_position();
  if (client_position > server_position)
  {
   std::vector<char> v(client_journal.get_raw_tail(server_position));

   if (trace)
    std::cerr << v.size() << " bytes... ";

   sftp_file file = sftp_open
   (
    sftp.get(),
    remote_file_name.c_str(),
    O_WRONLY | O_APPEND | O_CREAT,
    S_IRUSR | S_IWUSR
   );

   if (file)
   {
    ssize_t written = sftp_write(file, v.data(), v.size());
    sftp_close(file);
    if (written < ssize_t(v.size()))
     throw Exception("Incomplete write during push");
   }
   else
    throw Exception("Could not open remote file for writing");
  }

  if (trace)
   std::cerr << "done\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 SSH_Connection::SSH_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  std::string user,
  std::string host,
  int port,
  std::string remote_file_name,
  bool trace,
  int ssh_log_level
 ):
  remote_file_name(remote_file_name),
  trace(trace),
  mutex_file_name(remote_file_name + ".mutex"),
  full_remote_name(user + "@" + host + ":" + remote_file_name),
  session
  (
   user,
   host,
   port,
   ssh_log_level
  ),
  sftp(session),
  server_position(0)
 {
 }

#define RETRY(x, n)\
 for (int i = n; --i >= 0;)\
  try\
  {\
   x;\
   return;\
  }\
  catch (const Exception &e)\
  {\
   if (trace)\
    std::cerr << "Error: " << e.what() << '\n';\
   reset();\
  }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Robust_Connection::pull(Journal_File &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  RETRY(connection->pull(client_journal), 2);
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Robust_Connection::lock_pull(Journal_File &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  RETRY(connection->lock_pull(client_journal), 2);
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Robust_Connection::push_unlock(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  RETRY(connection->push_unlock(client_journal), 2);
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Robust_Connection::reset()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (trace && connection)
  {
   std::cerr << "An error occurred, trying to reset the connection...\n";
   std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  for (int i = 100; --i >= 0;)
   try
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
    return;
   }
   catch (const Exception &e)
   {
    if (trace)
    {
     std::cerr << "Connection failed: " << e.what() << '\n';
     std::cerr << "Will retry after a pause...\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
   }

  throw Exception("Impossible to connect. Giving up");
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
  ssh_log_level(ssh_log_level)
 {
  reset();
 }
}

#endif
