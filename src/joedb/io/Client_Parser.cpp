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
  out << " [--nocheck] <file> <connection>\n\n";

  file_parser.print_help(out);
  connection_parser.print_help(out);
 }

 ////////////////////////////////////////////////////////////////////////////
 Pullonly_Client &Client_Parser::parse(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 0;

  bool content_check = true;
  if (arg_index < argc && std::strcmp(argv[arg_index], "--nocheck") == 0)
  {
   arg_index++;
   content_check = false;
  }
  std::cerr << "content_check = " << content_check << '\n';

  Generic_File &client_file = file_parser.parse
  (
   std::cerr,
   argc,
   argv,
   arg_index
  );

  std::cerr << "Creating client data... ";

  if (client_file.get_mode() == Open_Mode::read_existing)
   client_data.reset(new Readonly_Interpreted_Client_Data(client_file));
  else
   client_data.reset(new Writable_Interpreted_Client_Data(client_file));

  std::cerr << "OK\n";

  Pullonly_Connection &pullonly_connection = connection_parser.build
  (
   argc - arg_index,
   argv + arg_index
  );

  Connection *push_connection = pullonly_connection.get_push_connection();

  if (push_connection)
   client.reset(new Client(*client_data, *push_connection, content_check));
  else
   client.reset(new Pullonly_Client(*client_data, pullonly_connection, content_check));

  return *client;
 }
}
