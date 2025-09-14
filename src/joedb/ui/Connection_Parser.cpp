#include "joedb/ui/Connection_Parser.h"
#include "joedb/ui/Connection_Builder.h"
#include "joedb/ui/Dummy_Connection_Builder.h"
#include "joedb/ui/File_Connection_Builder.h"
#include "joedb/ui/SQL_Dump_Writable.h"

#ifdef JOEDB_HAS_ASIO
#include "joedb/ui/Local_Connection_Builder.h"
#endif

#ifdef JOEDB_HAS_SSH
#include "joedb/ui/SSH_Connection_Builder.h"
#endif

#ifdef JOEDB_HAS_WEBSOCKETS
#include "joedb/ui/Websocket_Connection_Builder.h"
#endif

#include <cstring>
#include <sstream>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Connection_Parser::Connection_Parser()
 //////////////////////////////////////////////////////////////////////////
 {
  builders.emplace_back(new Dummy_Connection_Builder());
  builders.emplace_back(new File_Connection_Builder());

#ifdef JOEDB_HAS_ASIO
  builders.emplace_back(new Local_Connection_Builder());
#endif

#ifdef JOEDB_HAS_SSH
  builders.emplace_back(new SSH_Connection_Builder());
#endif

#ifdef JOEDB_HAS_WEBSOCKETS
  builders.emplace_back(new Websocket_Connection_Builder());
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
 Connection_Builder &Connection_Parser::get_builder(std::string_view name) const
 //////////////////////////////////////////////////////////////////////////
 {
  for (const auto &b: builders)
   if (name == b->get_name())
    return *b;

  std::ostringstream message;
  message << "Unknown connection type: " << name << '\n';
  print_help(message);
  throw Exception(message.str());
 }

 //////////////////////////////////////////////////////////////////////////
 Connection *Connection_Parser::build
 //////////////////////////////////////////////////////////////////////////
 (
  Arguments &arguments,
  Abstract_File *file
 ) const
 {
  std::string_view connection_name;

  if (arguments.get_remaining_count() == 0)
   connection_name = builders[0]->get_name();
  else
   connection_name = arguments.get_next();

  std::cerr << "Creating connection (" << connection_name << ") ... ";

  Connection *result = get_builder(connection_name.data()).build
  (
   arguments,
   file
  );

  std::cerr << "OK\n";

  return result;
 }
}
