#include "joedb/server/SSH_Connection.h"
#include "joedb/Exception.h"

#include <iostream>
#include <fcntl.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::run(const std::string &command)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "running: " << command << "... ";
  ssh::Channel channel(session);
  channel.request_exec(command.c_str());
  const int exit_status = channel.get_exit_status();
  std::cerr << "exit status: " << exit_status << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  // TODO: avoid code injection
  run("lockfile -1 " + remote_file_name + ".mutex");
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (sftp_unlink(sftp.get(), (remote_file_name + ".mutex").c_str()) < 0)
   throw Exception("Error removing remote mutex");
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::pull(Journal_File &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  ssh::SFTP_Attributes attributes
  (
   sftp_stat(sftp.get(), remote_file_name.c_str())
  );

  server_position = int64_t(attributes.get()->size);

  const int64_t client_position = client_journal.get_checkpoint_position();

  if (client_position < server_position)
  {
   std::vector<char> v(size_t(server_position - client_position));
   std::cerr << "copying " << v.size() << " bytes from server.\n";

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
 }

 ////////////////////////////////////////////////////////////////////////////
 void SSH_Connection::push(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t client_position = client_journal.get_checkpoint_position();
  if (client_position > server_position)
  {
   std::vector<char> v(client_journal.get_raw_tail(server_position));

   std::cerr << "copying " << v.size() << " bytes to server.\n";

   sftp_file file = sftp_open
   (
    sftp.get(),
    remote_file_name.c_str(),
    O_WRONLY | O_APPEND,
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
 }

 ////////////////////////////////////////////////////////////////////////////
 SSH_Connection::SSH_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  std::string user,
  std::string host,
  int port,
  std::string remote_file_name
 ):
  remote_file_name(remote_file_name),
  session
  (
   user,
   host,
   port,
//   SSH_LOG_PACKET
//   SSH_LOG_PROTOCOL
   SSH_LOG_NOLOG
  ),
  sftp(session),
  server_position(0)
 {
 }
}
