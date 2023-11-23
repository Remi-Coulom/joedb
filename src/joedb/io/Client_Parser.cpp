#include "joedb/io/Client_Parser.h"
#include "joedb/io/Connection_Parser.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/SFTP_File.h"
#include "joedb/concurrency/Writable_Journal_Client_Data.h"
#include "joedb/concurrency/Readonly_Journal_Client_Data.h"
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
  out << "  [--shared|--exclusive] <client_file_name>\n";
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
   arg_index++;
   nodb = true;
  }

  if (arg_index + 3 < argc && std::strcmp(argv[arg_index], "sftp") == 0)
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
  else if (arg_index < argc && std::strcmp(argv[arg_index], "memory") == 0)
  {
   client_file.reset(new Memory_File());
   arg_index++;
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

   std::cout << "Opening local file... ";
   std::cout.flush();

   if (file_name && *file_name)
   {
    client_file.reset(new File(file_name, open_mode));
    std::cout << "OK\n";
   }
   else
    std::cout << "Error: missing file name\n";
  }

  std::cout << "Creating client data... ";
  std::cout.flush();

  if (nodb)
  {
   if (client_file->get_mode() == Open_Mode::read_existing)
    client_data.reset(new Readonly_Journal_Client_Data(*client_file));
   else
    client_data.reset(new Writable_Journal_Client_Data(*client_file));
  }
  else
  {
   if (client_file->get_mode() == Open_Mode::read_existing)
    client_data.reset(new Readonly_Interpreted_Client_Data(*client_file));
   else
    client_data.reset(new Writable_Interpreted_Client_Data(*client_file));
  }

  std::cout << "OK\n";

  std::cout << "Creating connection... ";
  std::cout.flush();

  connection = connection_parser.build(argc - arg_index, argv + arg_index);

  std::cout << "OK\n";

  client.reset(new Client(*client_data, *connection));

  return *client;
 }
}
