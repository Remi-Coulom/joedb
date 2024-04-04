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
  out << "<file> is one of:\n";

  out << " [file] [--<open_mode>] <file_name>\n";
  out << " <open_mode> is one of:\n";

  for (size_t i = 0; i < open_mode_strings.size(); i++)
  {
   out << "  " << open_mode_strings[i];
   if (Open_Mode(i) == default_open_mode)
    out << " (default)";
   out << '\n';
  }

#ifdef JOEDB_HAS_SSH
  out << " sftp [--port p] [--verbosity v] <user> <host> <file_name>\n";
#endif
  out << " memory\n";
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

  if (arg_index < argc && std::strcmp(argv[arg_index], "memory") == 0)
  {
   client_file.reset(new Memory_File());
   arg_index++;
  }
#ifdef JOEDB_HAS_SSH
  else if (arg_index + 3 < argc && std::strcmp(argv[arg_index], "sftp") == 0)
  {
   arg_index++;

   unsigned port = 22;
   if (arg_index + 4 < argc && std::strcmp(argv[arg_index], "--port") == 0)
   {
    arg_index++;
    port = uint16_t(std::atoi(argv[arg_index++]));
   }

   int verbosity = 0;
   if (arg_index + 4 < argc && std::strcmp(argv[arg_index], "--verbosity") == 0)
   {
    arg_index++;
    verbosity = std::atoi(argv[arg_index++]);
   }

   const char * const user = argv[arg_index++];
   const char * const host = argv[arg_index++];

   std::cout << "Creating ssh Session... ";
   std::cout.flush();

   ssh_session.reset(new ssh::Session(user, host, port, verbosity));

   std::cout << "OK\n";

   std::cout << "Initializing sftp... ";
   std::cout.flush();

   sftp.reset(new ssh::SFTP(*ssh_session));

   std::cout << "OK\n";

   const char * const file_name = argv[arg_index++];

   std::cout << "Opening file... ";

   client_file.reset(new SFTP_File(*sftp, file_name));

   std::cout << "OK\n";
  }
#endif
  else
  {
   if (arg_index < argc && std::strcmp(argv[arg_index], "file") == 0)
    arg_index++;

   joedb::Open_Mode open_mode = default_open_mode;

   for (size_t i = 0; i < open_mode_strings.size(); i++)
   {
    const std::string option = std::string("--") + open_mode_strings[i];
    if (arg_index < argc && option == argv[arg_index])
    {
     open_mode = Open_Mode(i);
     arg_index++;
    }
   }

   const char *file_name = nullptr;
   if (arg_index < argc)
   {
    file_name = argv[arg_index];
    arg_index++;
   }

   std::cout << "Opening local file (open_mode = ";
   std::cout << open_mode_strings[size_t(open_mode)] << ") ... ";
   std::cout.flush();

   if (file_name && *file_name)
   {
    client_file.reset(new File(file_name, open_mode));
    std::cout << "OK\n";
   }
   else
    throw Runtime_Error("missing file name");
  }

  std::cout << "Creating client data (has_db = " << has_db << ") ... ";
  std::cout.flush();

  if (has_db)
  {
   if (client_file->get_mode() == Open_Mode::read_existing)
    client_data.reset(new Readonly_Interpreted_Client_Data(*client_file));
   else
    client_data.reset(new Writable_Interpreted_Client_Data(*client_file));
  }
  else
  {
   if (client_file->get_mode() == Open_Mode::read_existing)
    client_data.reset(new Readonly_Journal_Client_Data(*client_file));
   else
    client_data.reset(new Writable_Journal_Client_Data(*client_file));
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
