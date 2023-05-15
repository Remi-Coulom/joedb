#include "joedb/io/main_exception_catcher.h"
#include "joedb/db/multi_server_interpreted.h"
#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Backup_Client.h"
#include "joedb/concurrency/Server_Connection.h"
#include "joedb/ssh/Forward_Channel.h"
#include "joedb/journal/Interpreted_File.h"

#include <iostream>
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
  private:
   File file;
   Writable_Journal journal;
   Server server;

  public:
   Server_Data
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
     &std::cerr
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
   auto ssh_connection = db.get_ssh_connection(server);

   if (ssh_connection.is_null())
   {
    if (!db.is_valid(ssh_connection))
     throw std::runtime_error("invlid ssh_connection id");
   }

   servers.emplace_back
   (
    new Server_Data
    (
     io_context,
     db.get_file_name(server),
     db.get_port(server),
     db.get_timeout(server)
    )
   );
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
