#include "joedb/ui/main_exception_catcher.h"
#include "joedb/db/multi_server/Readonly_Database.h"
#include "joedb/journal/Readonly_Interpreted_File.h"
#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/IO_Context_Wrapper.h"

#include <iostream>
#include <list>
#include <memory>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   File file;
   Connection connection;
   Writable_Journal_Client client;
   Server server;

  public:
   Server_Data
   (
    asio::io_context &io_context,
    const std::string &file_name,
    uint16_t port,
    std::chrono::seconds timeout
   ):
    file(file_name, Open_Mode::write_existing_or_create_new),
    client(file, connection),
    server
    (
     client,
     false,
     io_context,
     port,
     timeout,
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
  db::multi_server::Readonly_Database db
  (
   Readonly_Interpreted_File{config_file_name}
  );

  IO_Context_Wrapper io_context_wrapper;

  std::list<std::unique_ptr<Server_Data>> servers;

  for (auto server: db.get_server_table())
  {
   servers.emplace_back
   (
    new Server_Data
    (
     io_context_wrapper.io_context,
     db.get_file_name(server),
     uint16_t(db.get_port(server)),
     std::chrono::seconds(db.get_timeout(server))
    )
   );
  }

  io_context_wrapper.run();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
