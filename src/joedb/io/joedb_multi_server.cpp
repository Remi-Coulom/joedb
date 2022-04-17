#include "joedb/io/main_exception_catcher.h"
#include "joedb/db/multi_server.h"
#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Backup_Client.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/ssh/Forward_Channel.h"
#include "joedb/journal/Interpreted_File.h"

#include <iostream>
#include <joedb/db/multi_server_readonly.h>
#include <joedb/ssh/Thread_Safe_Session.h>
#include <list>
#include <memory>
#include <fstream>
#include <stdexcept>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual ~Server_Data() = default;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Plain_Server_Data: public Server_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   File file;
   Writable_Journal journal;
   Server server;

  public:
   Plain_Server_Data
   (
    net::io_context &io_context,
    const std::string &file_name,
    int32_t port,
    int32_t timeout
   ):
    file(file_name, Open_Mode::write_existing_or_create_new),
    journal(file),
    server
    (
     journal,
     io_context,
     uint16_t(port),
     uint32_t(timeout),
     &std::cerr,
     nullptr
    )
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Server_Data_With_Backup: public Server_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   File file;
   ssh::Thread_Safe_Session session;
   ssh::Forward_Channel channel;
   Server_Connection backup_connection;
   Backup_Client backup_client;
   Server server;

  public:
   Server_Data_With_Backup
   (
    net::io_context &io_context,
    const std::string &file_name,
    int32_t port,
    int32_t timeout,
    const std::string &backup_user,
    const std::string &backup_host,
    const int32_t backup_joedb_port,
    const int32_t backup_ssh_port,
    const int32_t backup_ssh_log_level
   ):
    file(file_name, Open_Mode::write_existing_or_create_new),
    session(backup_user, backup_host, backup_ssh_port, backup_ssh_log_level),
    channel(session, "localhost", uint16_t(backup_joedb_port)),
    backup_connection(channel, &std::cerr),
    backup_client(backup_connection, file),
    server
    (
     backup_client.get_journal(),
     io_context,
     uint16_t(port),
     uint32_t(timeout),
     &std::cerr,
     &backup_client
    )
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (argc != 2)
  {
   std::cerr << "usage: " << argv[0] << " <config.joedbi>\n";
   return 1;
  }

  const char * const config_file_name = argv[1];
  multi_server::Interpreted_Database db(config_file_name);

  net::io_context io_context;

  std::list<std::unique_ptr<Server_Data>> servers;

  for (auto server: db.get_server_table())
  {
   auto ssh_backup = db.get_ssh_backup(server);

   if (ssh_backup)
   {
    if (!db.is_valid(ssh_backup))
     throw std::runtime_error("invlid ssh_backup id");

    servers.emplace_back
    (
     new Server_Data_With_Backup
     (
      io_context,
      db.get_file_name(server),
      db.get_port(server),
      db.get_timeout(server),
      db.get_user(ssh_backup),
      db.get_host(ssh_backup),
      db.get_joedb_port(ssh_backup),
      db.get_ssh_port(ssh_backup),
      db.get_ssh_log_level(ssh_backup)
     )
    );
   }
   else
   {
    servers.emplace_back
    (
     new Plain_Server_Data
     (
      io_context,
      db.get_file_name(server),
      db.get_port(server),
      db.get_timeout(server)
     )
    );
   }
  }

  io_context.run();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
