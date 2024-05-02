#include "joedb/io/Client_Parser.h"
#include "joedb/io/Connection_Parser.h"
#include "joedb/io/open_mode_strings.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/concurrency/Writable_Journal_Client_Data.h"
#include "joedb/concurrency/Readonly_Journal_Client_Data.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Client.h"

#ifdef JOEDB_HAS_SSH
#include "joedb/ssh/Session.h"
#include "joedb/journal/SFTP_File.h"
#endif

#include <iostream>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Client_Parser::Client_Parser(bool local, Open_Mode default_open_mode):
 ////////////////////////////////////////////////////////////////////////////
  file_parser(default_open_mode),
  connection_parser(local),
  default_open_mode(default_open_mode)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Parser::print_help(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << " <file> <connection>\n\n";

  file_parser.print_help(out);
  connection_parser.print_help(out);
 }

 ////////////////////////////////////////////////////////////////////////////
 Pullonly_Client &Client_Parser::parse(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 0;

  Generic_File &client_file = file_parser.parse
  (
   std::cout,
   argc,
   argv,
   arg_index
  );

  std::cout << "Creating client data... ";
  std::cout.flush();

  if (client_file.get_mode() == Open_Mode::read_existing)
   client_data.reset(new Readonly_Interpreted_Client_Data(client_file));
  else
   client_data.reset(new Writable_Interpreted_Client_Data(client_file));

  std::cout << "OK\n";

  Pullonly_Connection &pullonly_connection = connection_parser.build
  (
   argc - arg_index,
   argv + arg_index
  );

  Connection *connection = pullonly_connection.get_connection();

  if (connection)
   client.reset(new Client(*client_data, *connection));
  else
   client.reset(new Pullonly_Client(*client_data, pullonly_connection));

  return *client;
 }
}
