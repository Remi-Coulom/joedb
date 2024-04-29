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
 Client_Parser::Client_Parser
 ////////////////////////////////////////////////////////////////////////////
 (
  bool local,
  bool default_has_db,
  Open_Mode default_open_mode
 ):
  file_parser(default_open_mode),
  connection_parser(local),
  default_has_db(default_has_db),
  default_open_mode(default_open_mode)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Parser::print_help(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  if (default_has_db)
   out << " [--nodb]";
  else
   out << " [--db]";

  out << " <file> <connection>\n\n";

  file_parser.print_help(out);
  connection_parser.print_help(out);
 }

 ////////////////////////////////////////////////////////////////////////////
 Client &Client_Parser::parse(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 0;

  bool has_db = default_has_db;

  if (arg_index < argc && std::strcmp(argv[arg_index], "--nodb") == 0)
  {
   arg_index++;
   has_db = false;
  }

  if (arg_index < argc && std::strcmp(argv[arg_index], "--db") == 0)
  {
   arg_index++;
   has_db = true;
  }

  Generic_File &client_file = file_parser.parse
  (
   std::cout,
   argc,
   argv,
   arg_index
  );

  std::cout << "Creating client data (has_db = " << has_db << ") ... ";
  std::cout.flush();

  if (has_db)
  {
   if (client_file.get_mode() == Open_Mode::read_existing)
    client_data.reset(new Readonly_Interpreted_Client_Data(client_file));
   else
    client_data.reset(new Writable_Interpreted_Client_Data(client_file));
  }
  else
  {
   if (client_file.get_mode() == Open_Mode::read_existing)
    client_data.reset(new Readonly_Journal_Client_Data(client_file));
   else
    client_data.reset(new Writable_Journal_Client_Data(client_file));
  }

  std::cout << "OK\n";

  connection = connection_parser.build
  (
   argc - arg_index,
   argv + arg_index
  );

  client.reset(new Client(*client_data, *connection));

  return *client;
 }
}
