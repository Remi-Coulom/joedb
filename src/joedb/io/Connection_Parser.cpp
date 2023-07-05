#include "joedb/io/Connection_Parser.h"
#include "joedb/io/Connection_Builder.h"
#include "joedb/io/Dump_Connection_Builder.h"
#include "joedb/io/Dummy_Connection_Builder.h"
#include "joedb/io/File_Connection_Builder.h"
#include "joedb/journal/File.h"

#ifdef JOEDB_FILE_IS_LOCKABLE
#include "joedb/io/Local_Connection_Builder.h"
#endif

#ifdef JOEDB_HAS_ASIO_NET
#include "joedb/io/Network_Connection_Builder.h"
#endif

#ifdef JOEDB_HAS_SSH
#include "joedb/io/SSH_Connection_Builder.h"
#include "joedb/io/SFTP_Connection_Builder.h"
#endif

#include <cstring>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Connection_Parser::Connection_Parser(bool local)
 //////////////////////////////////////////////////////////////////////////
 {
  builders.emplace_back(new Dump_Connection_Builder());
  builders.emplace_back(new Tail_Connection_Builder());

  if (local)
  {
   builders.emplace_back(new Dummy_Connection_Builder());
#ifdef JOEDB_FILE_IS_LOCKABLE
   builders.emplace_back(new Local_Connection_Builder());
#endif
  }

  builders.emplace_back(new File_Connection_Builder());

#ifdef JOEDB_HAS_ASIO_NET
  builders.emplace_back(new Network_Connection_Builder());
#endif

#ifdef JOEDB_HAS_SSH
  builders.emplace_back(new SSH_Connection_Builder());
  builders.emplace_back(new SFTP_Connection_Builder());
#endif
 }

 //////////////////////////////////////////////////////////////////////////
 void Connection_Parser::list_builders() const
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
 Connection_Builder *Connection_Parser::get_builder(const char *name) const
 //////////////////////////////////////////////////////////////////////////
 {
  Connection_Builder *builder = nullptr;

  for (const auto &b: builders)
  {
   if (std::strcmp(b->get_name(), name) == 0)
   {
    builder = b.get();
    break;
   }
  }

  if (builder == nullptr)
  {
   std::cerr << "Unknown connection type: " << name << '\n';
   list_builders();
  }

  return builder;
 }

 //////////////////////////////////////////////////////////////////////////
 std::unique_ptr<Connection> Connection_Parser::build
 //////////////////////////////////////////////////////////////////////////
 (
  Connection_Builder &builder,
  int argc,
  char **argv
 )
 {
  if
  (
   argc < builder.get_min_parameters() ||
   argc > builder.get_max_parameters()
  )
  {
   std::cerr << "Wrong number of parameters. Expected: ";
   std::cerr << builder.get_parameters_description() << '\n';
   return nullptr;
  }
  else
   return builder.build(argc, argv);
 }

 //////////////////////////////////////////////////////////////////////////
 std::unique_ptr<Connection> Connection_Parser::build
 //////////////////////////////////////////////////////////////////////////
 (
  int argc,
  char **argv
 ) const
 {
  if (argc > 0)
  {
   const char *name = argv[0];

   Connection_Builder *builder = get_builder(name);

   if (builder)
    return build(*builder, argc - 1, argv + 1);
  }

  return nullptr;
 }
}
