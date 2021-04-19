#ifdef JOEDB_HAS_SSH

#include "joedb/concurrency/SSH_Connection.h"
#include "joedb/ssh/Thread_Safe_Session.h"
#include "joedb/Exception.h"

#include <thread>
#include <chrono>
#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int64_t SSH_Connection::raw_pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  ssh::Session_Lock lock(remote_mutex.session);

  if (remote_mutex.trace)
   std::cerr << remote_mutex.full_remote_name << ": pull()... ";

  int64_t server_position = 0;

  try
  {
   ssh::SFTP_Attributes attributes
   (
    sftp_stat(lock.get_sftp_session(), remote_mutex.remote_file_name.c_str())
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

   if (remote_mutex.trace)
    std::cerr << v.size() << " bytes... ";

   sftp_file file = sftp_open
   (
    lock.get_sftp_session(),
    remote_mutex.remote_file_name.c_str(),
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
     if (remote_mutex.trace)
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

  if (remote_mutex.trace)
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
  ssh::Session_Lock lock(remote_mutex.session);

  if (remote_mutex.trace)
   std::cerr << remote_mutex.full_remote_name << ": push()... ";

  const int64_t client_position = client_journal.get_checkpoint_position();
  if (client_position > server_position)
  {
   std::vector<char> v(client_journal.get_raw_tail(server_position));

   if (remote_mutex.trace)
    std::cerr << v.size() << " bytes... ";

   sftp_file file = sftp_open
   (
    lock.get_sftp_session(),
    remote_mutex.remote_file_name.c_str(),
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

  if (remote_mutex.trace)
   std::cerr << "done.\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 SSH_Connection::SSH_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  ssh::Thread_Safe_Session &session,
  std::string remote_file_name,
  bool trace
 ):
  remote_mutex(session, remote_file_name, trace),
  keepalive_thread(session)
 {
 }
}

#endif
