#include "joedb/io/Connection_Parser.h"
#include "joedb/io/Connection_Builder.h"
#include "joedb/io/Dump_Connection_Builder.h"
#include "joedb/io/Dummy_Connection_Builder.h"
#include "joedb/io/File_Connection_Builder.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/io/SQL_Dump_Writable.h"

#ifdef JOEDB_HAS_NETWORKING
#include "joedb/io/Network_Connection_Builder.h"
#endif

#ifdef JOEDB_HAS_SSH
#include "joedb/io/SSH_Connection_Builder.h"
#endif

#include <cstring>
#include <sstream>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Connection_Parser::Connection_Parser(bool local, bool use_server_file)
 //////////////////////////////////////////////////////////////////////////
 {
  if (local)
   builders.emplace_back(new Dummy_Connection_Builder());

  builders.emplace_back
  (
   new Dump_Connection_Builder<Interpreter_Writable>(std::cout)
  );

  builders.emplace_back
  (
   new Dump_Connection_Builder<SQL_Writable>(std::cout)
  );

  builders.emplace_back(new File_Connection_Builder());

#ifdef JOEDB_HAS_NETWORKING
  builders.emplace_back(new Network_Connection_Builder(false));
  if (use_server_file)
   builders.emplace_back(new Network_Connection_Builder(true));
#endif

#ifdef JOEDB_HAS_SSH
  builders.emplace_back(new SSH_Connection_Builder(false));
  if (use_server_file)
   builders.emplace_back(new SSH_Connection_Builder(true));
#endif
 }

 //////////////////////////////////////////////////////////////////////////
 void Connection_Parser::print_help(std::ostream &out) const
 //////////////////////////////////////////////////////////////////////////
 {
  out << "\n<connection> is one of:\n";
  for (size_t i = 0; i < builders.size(); i++)
  {
   out << ' ';

   if (i == 0)
    out << '[';

   out<< builders[i]->get_name();

   if (i == 0)
    out << "] (default)";

   out << ' ' << builders[i]->get_parameters_description();
   out << '\n';
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
 Pullonly_Connection &Connection_Parser::build
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
 Pullonly_Connection &Connection_Parser::build
 //////////////////////////////////////////////////////////////////////////
 (
  int argc,
  char **argv
 ) const
 {
  const char * connection_name;
  if (argc <= 0)
  {
   argc = 1;
   connection_name = builders[0]->get_name();
  }
  else
   connection_name = argv[0];

  std::cerr << "Creating connection (" << connection_name << ") ... ";

  Pullonly_Connection &result = build
  (
   get_builder(connection_name),
   argc - 1,
   argv + 1
  );

  std::cerr << "OK\n";

  return result;
 }
}
