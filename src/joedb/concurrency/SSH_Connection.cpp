#ifdef JOEDB_HAS_SSH

#include "joedb/concurrency/SSH_Connection.h"
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
 void SSH_Connection::keepalive()
 ////////////////////////////////////////////////////////////////////////////
 {
  ssh::Session_Lock lock(thread_safe_ssh_session);

  while (true)
  {
   keepalive_condition.wait_for
   (
    lock,
    std::chrono::seconds(keepalive_interval)
   );

   if (keepalive_thread_must_stop)
    break;
   else
    ssh_send_ignore(lock.get_ssh_session(), "keepalive");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  ssh::Session_Lock lock(thread_safe_ssh_session);

  if (trace)
   std::cerr << full_remote_name << ": lock()... ";

  bool done = false;
  const int max_attempts = 600;

  for (int attempt = 1; attempt <= max_attempts; attempt++)
  {
   sftp_file file = sftp_open
   (
    lock.get_sftp_session(),
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
  ssh::Session_Lock lock(thread_safe_ssh_session);

  if (trace)
   std::cerr << full_remote_name << ": unlock()\n";

  if (sftp_unlink(lock.get_sftp_session(), mutex_file_name.c_str()) < 0)
   throw Exception("Error removing remote mutex");
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t SSH_Connection::raw_pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  ssh::Session_Lock lock(thread_safe_ssh_session);

  if (trace)
   std::cerr << full_remote_name << ": pull()... ";

  int64_t server_position = 0;

  try
  {
   ssh::SFTP_Attributes attributes
   (
    sftp_stat(lock.get_sftp_session(), remote_file_name.c_str())
   );

   server_position = int64_t(attributes.get()->size);
  }
  catch (const joedb::Exception &)
  {
   throw Exception("Could not stat remote file");
  }

  const int64_t client_position = client_journal.get_checkpoint_position();

  if (client_position < server_position)
  {
   std::vector<char> v(size_t(server_position - client_position));

   if (trace)
    std::cerr << v.size() << " bytes... ";

   sftp_file file = sftp_open
   (
    lock.get_sftp_session(),
    remote_file_name.c_str(),
    O_RDONLY,
    S_IRUSR | S_IWUSR
   );

   if (file)
   {
    const int seek_result = sftp_seek64(file, uint64_t(client_position));
    if (seek_result < 0)
    {
     sftp_close(file);
     throw Exception("Error while seeking");
    }

    ssize_t total_read = 0;

    while (total_read < ssize_t(v.size()))
    {
     const ssize_t read_result = sftp_read
     (
      file,
      &v[size_t(total_read)],
      v.size() - size_t(total_read)
     );

     if (read_result < 0)
     {
      sftp_close(file);
      throw Exception("Error during sftp_read");
     }

     total_read += read_result;
     if (trace)
      std::cerr << read_result << ' ';
    }

    sftp_close(file);
    client_journal.append_raw_tail(v);
   }
   else
    throw Exception("Could not open remote file for reading");
  }
  else if (client_position > server_position)
   throw Exception("Trying to pull when ahead of server");

  if (trace)
   std::cerr << "done.\n";

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
  ssh::Session_Lock lock(thread_safe_ssh_session);

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
    lock.get_sftp_session(),
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
   std::cerr << "done.\n";
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
  thread_safe_ssh_session
  (
   user,
   host,
   port,
   ssh_log_level
  ),
  keepalive_thread([this](){keepalive();})
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 SSH_Connection::~SSH_Connection()
 ////////////////////////////////////////////////////////////////////////////
 {
  {
   ssh::Session_Lock lock(thread_safe_ssh_session);
   keepalive_thread_must_stop = true;
   keepalive_condition.notify_one();
  }
  keepalive_thread.join();
 }
}

#endif
