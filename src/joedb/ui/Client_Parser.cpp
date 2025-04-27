#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/concurrency/Readonly_Database_Client.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Readonly_Journal_Client.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Client.h"

#include <iostream>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Client_Parser::Client_Parser
 ////////////////////////////////////////////////////////////////////////////
 (
  bool local,
  Open_Mode default_open_mode,
  bool with_database
 ):
  file_parser(default_open_mode, false, true, true),
  connection_parser(local),
  default_open_mode(default_open_mode),
  default_with_database(with_database)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Parser::print_help(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << " [--nocheck]";
  if (default_with_database)
   out << " [--nodb]";
  out << " <file> <connection>\n\n";

  file_parser.print_help(out);
  connection_parser.print_help(out);
 }

 ////////////////////////////////////////////////////////////////////////////
 Client &Client_Parser::parse(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 0;

  Content_Check content_check = Content_Check::quick;
  if (arg_index < argc && std::strcmp(argv[arg_index], "--nocheck") == 0)
  {
   arg_index++;
   content_check = Content_Check::none;
  }
  std::cerr << "content_check = " << int(content_check) << '\n';

  bool with_database = default_with_database;
  if (arg_index < argc && std::strcmp(argv[arg_index], "--nodb") == 0)
  {
   arg_index++;
   with_database = false;
  }

  file_parser.parse
  (
   std::cerr,
   argc,
   argv,
   arg_index
  );

  Buffered_File *client_file = file_parser.get_file();

  Connection &connection = connection_parser.build
  (
   argc - arg_index,
   argv + arg_index,
   client_file
  );

  if (!client_file)
   client_file = dynamic_cast<Buffered_File *>(&connection);

  if (!client_file)
   throw Exception("server file must be used with a network or ssh connection");

  std::cerr << "Creating client data... ";
  std::cerr << "OK\n";

  if (with_database)
  {
   if (client_file->is_readonly())
    client.reset(new Readonly_Database_Client(*client_file, connection, content_check));
   else
    client.reset(new Writable_Database_Client(*client_file, connection, content_check));
  }
  else
  {
   if (client_file->is_readonly())
    client.reset(new Readonly_Journal_Client(*client_file, connection, content_check));
   else
    client.reset(new Writable_Journal_Client(*client_file, connection, content_check));
  }

  return *client;
 }
}
