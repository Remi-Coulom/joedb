#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/multi_server_readonly.h"
#include "joedb/concurrency/Server.h"

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
   Writable_Journal journal;
   Server server;

  public:
   Server_Data
   (
    net::io_context &io_context,
    const std::string file_name,
    int32_t port,
    int32_t timeout
   ):
    file(file_name, Open_Mode::write_existing_or_create_new),
    journal(file),
    server(journal, io_context, uint16_t(port), uint32_t(timeout))
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 int main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 2)
  {
   std::cerr << "usage: " << argv[0] << " <multi_server.joedb>\n";
   return 1;
  }

  net::io_context io_context;

  multi_server::Readonly_Database db(argv[1]);

  std::list<std::unique_ptr<Server_Data>> servers;

  for (auto server: db.get_server_table())
  {
   servers.push_back
   (
    std::make_unique<Server_Data>
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
