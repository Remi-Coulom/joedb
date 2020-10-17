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

  bool done = false;
  const int max_attempts = 60;

  for (int attempt = 1; attempt <= max_attempts; attempt++)
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
    done = true;
    sftp_close(file);
    break;
   }
   else
   {
    if (trace)
     std::cerr << "Retrying(" << attempt << "/" << max_attempts << ")... ";
    std::this_thread::sleep_for(std::chrono::seconds(1));
   }
  }

  if (trace)
  {
   if (done)
    std::cerr << "done.\n";
   else
    std::cerr << "timeout.\n";
  }

  if (!done)
   throw Exception("SSH_Connection::lock: timeout");
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
 int64_t SSH_Connection::raw_pull(Journal_File &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (trace)
   std::cerr << full_remote_name << ": pull()... ";

  int64_t server_position = 0;

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
    const int seek_result = sftp_seek64(file, uint64_t(client_position));
    const ssize_t read_result = sftp_read(file, v.data(), v.size());
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

  return server_position;
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::raw_push
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_position
 )
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
    O_WRONLY | O_CREAT,
    S_IRUSR | S_IWUSR
   );

   const int seek_result = sftp_seek64(file, uint64_t(server_position));

   if (file)
   {
    //
    // https://tools.ietf.org/html/draft-ietf-secsh-filexfer-00
    //
    // The maximum size of a packet is in practise determined by the client
    // (the maximum size of read or write requests that it sends, plus a few
    // bytes of packet overhead).  All servers SHOULD support packets of at
    // least 34000 bytes (where the packet size refers to the full length,
    // including the header above).  This should allow for reads and writes of
    // at most 32768 bytes.
    //
    const size_t max_block_size = 32768;
    const size_t size = v.size();
    size_t written = 0;

    while (seek_result >= 0 && written < size)
    {
     size_t block_size = size - written;

     if (block_size > max_block_size)
      block_size = max_block_size;

     const ssize_t result = sftp_write(file, v.data() + written, block_size);

     if (result <= 0)
      break;

     written += size_t(result);
    }

    sftp_close(file);

    if (written < size)
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
  sftp(session)
 {
 }

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

   while (true)
   {
    if (connection)
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

 ////////////////////////////////////////////////////////////////////////////
 int64_t SSH_Robust_Connection::pull(Journal_File &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return retry
  (
   [&client_journal,this](){return connection->pull(client_journal);}
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t SSH_Robust_Connection::lock_pull(Journal_File &client_journal)
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
  ssh_log_level(ssh_log_level)
 {
  retry([this]()->int64_t{return 0;});
 }
}

#endif
