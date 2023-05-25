#ifndef joedb_Connection_Parser_declared
#define joedb_Connection_Parser_declared

#include "joedb/io/Connection_Builder.h"
#include "joedb/io/Dump_Connection_Builder.h"
#include "joedb/io/Network_Connection_Builder.h"
#include "joedb/io/SSH_Connection_Builder.h"

#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::unique_ptr<Connection_Builder>> builders;

  public:
   //////////////////////////////////////////////////////////////////////////
   Connection_Parser()
   //////////////////////////////////////////////////////////////////////////
   {
    builders.emplace_back(new Dump_Connection_Builder());
    builders.emplace_back(new Network_Connection_Builder());
    builders.emplace_back(new SSH_Connection_Builder());
   }

   //////////////////////////////////////////////////////////////////////////
   void list_builders()
   //////////////////////////////////////////////////////////////////////////
   {
    std::cerr << "available connections:\n";
    for (const auto &builder: builders)
    {
     std::cerr << ' ' << builder->get_name() << ' ';
     std::cerr << builder->get_parameters_description() << '\n';
    }
   }

   //////////////////////////////////////////////////////////////////////////
   std::unique_ptr<Connection> build(int argc, char **argv)
   //////////////////////////////////////////////////////////////////////////
   {
    const char * type;

    if (argc == 0)
     type = "dump";
    else
    {
     type = argv[0];
     argc--;
     argv++;
    }

    Connection_Builder *builder = nullptr;

    for (const auto &b: builders)
    {
     if (std::strcmp(b->get_name(), type) == 0)
     {
      builder = b.get();
      break;
     }
    }

    if (builder == nullptr)
    {
     std::cerr << "unknown connection type\n";
     list_builders();
     return nullptr;
    }

    if
    (
     argc < builder->get_min_parameters() ||
     argc > builder->get_max_parameters()
    )
    {
     std::cerr << "Wrong number of parameters. Expected: ";
     std::cerr << builder->get_parameters_description() << '\n';
     return nullptr;
    }
    else
     return builder->build(argc, argv);

    return nullptr;
   }
 };
}

#endif
