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
#include <sstream>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Connection_Parser::Connection_Parser(bool local, bool readonly)
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
  if (readonly)
   builders.emplace_back(new SFTP_Connection_Builder());
#endif
 }

 //////////////////////////////////////////////////////////////////////////
 void Connection_Parser::print_help(std::ostream &out) const
 //////////////////////////////////////////////////////////////////////////
 {
  out << "\n<connection> is one of:\n";
  for (const auto &builder: builders)
  {
   out << ' ' << builder->get_name() << ' ';
   out << builder->get_parameters_description() << '\n';
  }
 }

 //////////////////////////////////////////////////////////////////////////
 Connection_Builder &Connection_Parser::get_builder(const char *name) const
 //////////////////////////////////////////////////////////////////////////
 {
  for (const auto &b: builders)
  {
   if (std::strcmp(b->get_name(), name) == 0)
    return *b;
  }

  std::ostringstream message;
  message << "Unknown connection type: " << name << '\n';
  print_help(message);
  throw Exception(message.str());
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
   const char * description = builder.get_parameters_description();
   if (!*description)
    description = "no parameters";
   throw Exception
   (
    std::string("Wrong number of connection arguments. Expected: ") +
    std::string(description)
   );
  }

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
  if (argc <= 0)
   throw Exception("Missing connection argument");

  return build(get_builder(argv[0]), argc - 1, argv + 1);
 }
}
