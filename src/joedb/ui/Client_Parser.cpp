#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Client.h"

#include <iostream>
#include <cstring>

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 Client_Parser::Client_Parser(bool local, Open_Mode default_open_mode):
 ////////////////////////////////////////////////////////////////////////////
  file_parser(default_open_mode, false, true, true),
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
 concurrency::Pullonly_Client &Client_Parser::parse(int argc, char **argv)
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

  file_parser.parse
  (
   std::cerr,
   argc,
   argv,
   arg_index
  );

  Buffered_File *client_file = file_parser.get_file();

  concurrency::Pullonly_Connection &pullonly_connection = connection_parser.build
  (
   argc - arg_index,
   argv + arg_index,
   client_file
  );

  concurrency::Connection *push_connection = pullonly_connection.get_push_connection();

  if (!client_file)
   client_file = dynamic_cast<Buffered_File *>(&pullonly_connection);

  if (!client_file)
   throw error::Exception("server file must be used with a network or ssh connection");

  std::cerr << "Creating client data... ";

  if (client_file->is_readonly())
   client_data.reset(new concurrency::Readonly_Interpreted_Client_Data(*client_file));
  else
   client_data.reset(new concurrency::Writable_Interpreted_Client_Data(*client_file));

  std::cerr << "OK\n";

  if (push_connection)
   client.reset(new concurrency::Client(*client_data, *push_connection, content_check));
  else
   client.reset(new concurrency::Pullonly_Client(*client_data, pullonly_connection, content_check));

  return *client;
 }
}
