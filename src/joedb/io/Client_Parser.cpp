#include "joedb/io/Client_Parser.h"
#include "joedb/io/Connection_Parser.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/SFTP_File.h"
#include "joedb/concurrency/Journal_Client_Data.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Client.h"
#include "joedb/ssh/Session.h"
#include "joedb/ssh/SFTP.h"

#include <iostream>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Client_Parser::Client_Parser(bool local, bool readonly):
 ////////////////////////////////////////////////////////////////////////////
  connection_parser(local, readonly)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Parser::print_help(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << " [--nodb] <file> <connection>\n\n";
  out << "<file> is one of:\n";
  out << "  [file] [--shared|--exclusive] <client_file_name>\n";
  out << "  sftp [--port p] [--verbosity v] <user> <host> <file_name>\n";
  out << "  memory\n";
  connection_parser.print_help(out);
 }

 ////////////////////////////////////////////////////////////////////////////
 Client &Client_Parser::parse(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 1;

  bool nodb = false;
  if (arg_index < argc && std::strcmp(argv[arg_index], "--nodb") == 0)
  {
   nodb = true;
   arg_index++;
  }

  if (arg_index + 3 < argc && std::strcmp(argv[arg_index], "sftp") == 0)
  {
   const char * const user = argv[arg_index + 1];
   const char * const host = argv[arg_index + 2];
   const unsigned port = 22;
   const int verbosity = 0;

   std::cout << "Creating ssh Session... ";
   std::cout.flush();

   ssh_session.reset(new ssh::Session(user, host, port, verbosity));

   std::cout << "OK\n";

   std::cout << "Initializing sftp... ";
   std::cout.flush();

   sftp.reset(new ssh::SFTP(*ssh_session));

   std::cout << "OK\n";

   const char * const file_name = argv[arg_index + 3];

   std::cout << "Opening file... ";

   client_file.reset(new SFTP_File(*sftp, file_name));

   std::cout << "OK\n";

   arg_index += 4;
  }
  else
  {
   joedb::Open_Mode open_mode = Open_Mode::read_existing;
   if (arg_index < argc)
   {
    if (std::strcmp(argv[arg_index], "--shared") == 0)
    {
     open_mode = Open_Mode::shared_write;
     arg_index++;
    }
    else if (std::strcmp(argv[arg_index], "--exclusive") == 0)
    {
     open_mode = Open_Mode::write_existing_or_create_new;
     arg_index++;
    }
   }

   const char *file_name = nullptr;
   if (arg_index < argc)
   {
    file_name = argv[arg_index];
    arg_index++;
   }

   std::cout << "Creating local file... ";
   std::cout.flush();

   if (file_name && *file_name)
    client_file.reset(new File(file_name, open_mode));
   else
    client_file.reset(new Memory_File());

   std::cout << "OK\n";
  }

  std::cout << "Creating client data... ";
  std::cout.flush();

  if (nodb)
   client_data.reset(new Journal_Client_Data(*client_file));
  else
   client_data.reset(new Interpreted_Client_Data(*client_file));

  std::cout << "OK\n";

  std::cout << "Creating connection... ";
  std::cout.flush();

  connection = connection_parser.build(argc - arg_index, argv + arg_index);

  std::cout << "OK\n";

  client.reset(new Client(*client_data, *connection));

  return *client;
 }
}
