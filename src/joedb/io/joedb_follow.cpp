#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Connection_Builder.h"
#include "joedb/io/Network_Connection_Builder.h"
#include "joedb/io/Dump_Connection.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"

#include <thread>
#include <chrono>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dump_Connection_Builder: public Connection_Builder
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   std::unique_ptr<Connection> build(int argc, char **argv) final
   {
    return std::unique_ptr<Connection>(new Dump_Connection());
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 static int main(const int argc, char ** const argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 2)
  {
   std::cerr << "usage: " << argv[0] << " <file_name> [<connection> ...]\n";
   return 1;
  }
  else
  {
   const char * const file_name = argv[1];
   File file(file_name, Open_Mode::read_existing);
   Readonly_Journal journal(file);

   std::unique_ptr<Connection_Builder> builder;

   int builder_argc = argc - 2;
   char **builder_argv = argv + 2;

   if (argc == 2)
    builder.reset(new Dump_Connection_Builder());
   else
   {
    builder_argc--;
    builder_argv++;

    const std::string type = argv[2];

    if (type == "network")
    {
     builder.reset(new Network_Connection_Builder());
    }

    if (builder == 0)
    {
     std::cerr << "Error: Unknown connection type: " << type << '\n';
     return 1;
    }
   }

   if
   (
    builder_argc < builder->get_min_parameters() ||
    builder_argc > builder->get_max_parameters()
   )
   {
    std::cerr << "Parameters: ";
    std::cerr << builder->get_parameters_description() << '\n';
    return 1;
   }
   else
   {
    std::unique_ptr<Connection> connection = builder->build
    (
     builder_argc,
     builder_argv
    );

    int64_t server_checkpoint = connection->handshake(journal);

    connection->lock(journal);

    while (true)
    {
     std::this_thread::sleep_for(std::chrono::seconds(1));
     journal.refresh_checkpoint();
     const int64_t new_checkpoint = journal.get_checkpoint_position();

     if (new_checkpoint > server_checkpoint)
     {
      connection->push(journal, server_checkpoint, false);
      server_checkpoint = new_checkpoint;
     }
    }
   }
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
