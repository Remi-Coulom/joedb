#include "joedb/server/Good_SSH_Connection.h"
#include "joedb/Exception.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Good_SSH_Connection::run(const std::string &command)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "running: " << command << "... ";
  ssh::Channel channel(session);
  channel.request_exec(command.c_str());
  const int exit_status = channel.get_exit_status();
  std::cerr << "exit status: " << exit_status << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Good_SSH_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  // TODO: avoid code injection
  run("lockfile -1 " + remote_file_name + ".mutex");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Good_SSH_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  // TODO: avoid code injection
  run("rm -f " + remote_file_name + ".mutex");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Good_SSH_Connection::pull(Journal_File &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cerr << "pull\n";
  ssh::SFTP sftp(session);

  std::cerr << "getting attributes of remote file\n";
  ssh::SFTP_Attributes attributes
  (
   sftp_stat(sftp.get(), remote_file_name.c_str())
  );

  server_position = int64_t(attributes.get()->size);
  std::cerr << "server_position = " << server_position << '\n';

  const int64_t client_position = client_journal.get_checkpoint_position();

  if (client_position < server_position)
  {
   std::vector<char> v(size_t(server_position - client_position));

   // TODO: copy the tail of the server file to v

   client_journal.append_raw_tail
   (
    v
   );
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Good_SSH_Connection::push(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t client_position = client_journal.get_checkpoint_position();
  if (client_position > server_position)
  {
   ssh::SFTP sftp(session);
   std::vector<char> v(client_journal.get_raw_tail(server_position));
   // TODO: append this data to the server file
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Good_SSH_Connection::Good_SSH_Connection
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
  server_position(0)
 {
 }
}
