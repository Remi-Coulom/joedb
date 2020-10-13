#include "joedb/server/SSH_Connection.h"
#include "joedb/Exception.h"

#include <fcntl.h>
#include <thread>
#include <chrono>
#include <iostream>

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
    S_IRWXU
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
 void SSH_Connection::pull(Journal_File &client_journal)
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
  catch (const joedb::Exception &e)
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
    S_IRWXU
   );

   if (file)
   {
    sftp_seek64(file, uint64_t(client_position));
    sftp_read(file, v.data(), v.size());
    sftp_close(file);

    client_journal.append_raw_tail
    (
     v
    );
   }
   else
    throw Exception("Could not open remote file for reading");
  }

  if (trace)
   std::cerr << "done\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::push(Readonly_Journal &client_journal)
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
    S_IRWXU
   );

   if (file)
   {
    sftp_write(file, v.data(), v.size());
    sftp_close(file);
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
  bool trace
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
//   SSH_LOG_PROTOCOL
   SSH_LOG_NOLOG
  ),
  sftp(session),
  server_position(0)
 {
 }
}
