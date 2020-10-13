#include "joedb/server/System_SSH_Connection.h"
#include "joedb/journal/File.h"

#include <sstream>
#include <thread>
#include <chrono>
#include <iostream>

namespace joedb
{
 const std::string System_SSH_Connection::local_file_name
 (
  "SSH_Server_copy.joedb"
 );

 ////////////////////////////////////////////////////////////////////////////
 void System_SSH_Connection::run(const std::string &command)
 ////////////////////////////////////////////////////////////////////////////
 {
  while (true)
  {
   std::cerr << "run: " << command << '\n';
   const int result = std::system(command.c_str());
   if (result == 0)
    break;
   std::cerr << "execution result: " << result << '\n';
   std::this_thread::sleep_for(std::chrono::minutes(1));
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void System_SSH_Connection::lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  run("ssh -q -t -t " + host + " lockfile -1 " + remote_file_name + ".mutex");
 }

 ////////////////////////////////////////////////////////////////////////////
 void System_SSH_Connection::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  server_journal.reset();
  server_file.reset();
  run("ssh " + host + " rm -f " + remote_file_name + ".mutex");
 }

 ////////////////////////////////////////////////////////////////////////////
 void System_SSH_Connection::pull(Journal_File &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  run("rsync " + host + ":" + remote_file_name + ' ' + local_file_name);
  server_journal.reset();
  server_file.reset(new File(local_file_name, Open_Mode::write_existing));
  server_journal.reset(new Journal_File(*server_file));

  const int64_t client_position = client_journal.get_checkpoint_position();
  const int64_t server_position = server_journal->get_checkpoint_position();

  if (client_position < server_position)
   client_journal.append_raw_tail
   (
    server_journal->get_raw_tail(client_position)
   );
 }

 ////////////////////////////////////////////////////////////////////////////
 void System_SSH_Connection::push(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (server_journal)
  {
   const int64_t client_position = client_journal.get_checkpoint_position();
   const int64_t server_position = server_journal->get_checkpoint_position();

   if (server_position < client_position)
   {
    server_journal->append_raw_tail
    (
     client_journal.get_raw_tail(server_position)
    );

    server_journal.reset();
    server_file.reset();

    run("rsync " + local_file_name + ' ' + host + ":" + remote_file_name);
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 System_SSH_Connection::System_SSH_Connection
 ////////////////////////////////////////////////////////////////////////////
 (
  std::string host,
  std::string remote_file_name
 ):
  host(host),
  remote_file_name(remote_file_name)
 {
 }
}
