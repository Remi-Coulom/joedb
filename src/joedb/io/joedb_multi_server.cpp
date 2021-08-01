#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/multi_server_readonly.h"
#include "joedb/io/Interpreter.h"
#include "joedb/interpreter/Database.h"
#include "joedb/concurrency/Server.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/Readable_Multiplexer.h"

#include <iostream>
#include <list>
#include <memory>
#include <fstream>

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
  if (argc != 2)
  {
   std::cerr << "usage: " << argv[0] << " <config.joedbi>\n";
   return 1;
  }

  Memory_File memory_file(Open_Mode::create_new);
  Writable_Journal journal(memory_file);

  {
   Database db;
   Readable_Multiplexer multiplexer(db);
   multiplexer.add_writable(journal);
   Interpreter interpreter(multiplexer);
   std::ifstream joedbi_file(argv[1]);
   interpreter.set_echo(false);
   interpreter.main_loop(joedbi_file, std::cout);
   journal.checkpoint(0);
  }

  multi_server::Readonly_Database db(journal);

  net::io_context io_context;

  std::list<std::unique_ptr<Server_Data>> servers;

  for (auto server: db.get_server_table())
  {
   servers.push_back
   (
    std::unique_ptr<Server_Data>
    (
     new Server_Data
     (
      io_context,
      db.get_file_name(server),
      db.get_port(server),
      db.get_timeout(server)
     )
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
