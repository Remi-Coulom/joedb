#include "joedb/io/File_Parser.h"
#include "joedb/io/open_mode_strings.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"

#ifdef JOEDB_HAS_SSH
#include "joedb/journal/SFTP_File.h"
#endif

#ifdef JOEDB_HAS_CURL
#include "joedb/journal/CURL_File.h"
#endif

#ifdef JOEDB_HAS_BROTLI
#include "joedb/journal/Brotli_Codec.h"
#include "joedb/journal/Encoded_File.h"
#endif

#include <cstring>
#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void File_Parser::print_help(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "<file> is one of:\n";

  if (default_only)
  {
   out << " [file] <file_name>\n";
  }
  else
  {
   out << " [file] [--<open_mode>] <file_name>\n";
   out << " <open_mode> is one of:\n";

   for (size_t i = 0; i < open_mode_strings.size(); i++)
   {
    out << "  " << open_mode_strings[i];
    if (Open_Mode(i) == default_open_mode)
     out << " (default)";
    out << '\n';
   }

   out << " memory\n";
  }

#ifdef JOEDB_HAS_SSH
  out << " sftp [--port p] [--verbosity v] <user> <host> <file_name>\n";
#endif

#ifdef JOEDB_HAS_CURL
  out << " curl [--verbose] <URL>\n";
#endif

#ifdef JOEDB_HAS_BROTLI
  out << " brotli <file_name>\n";
#endif
 }

 ////////////////////////////////////////////////////////////////////////////
 Generic_File &File_Parser::parse
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  const int argc,
  char ** const argv,
  int &arg_index
 )
 {
  if (arg_index < argc && std::strcmp(argv[arg_index], "memory") == 0)
  {
   file.reset(new Memory_File());
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

   out << "Creating ssh Session... ";
   out.flush();

   ssh_session.reset(new ssh::Session(user, host, port, verbosity));

   out << "OK\n";

   out << "Initializing sftp... ";
   out.flush();

   sftp.reset(new ssh::SFTP(*ssh_session));

   out << "OK\n";

   const char * const file_name = argv[arg_index++];

   out << "Opening file... ";

   file.reset(new SFTP_File(*sftp, file_name));

   out << "OK\n";
  }
#endif
#ifdef JOEDB_HAS_CURL
  else if (arg_index < argc && std::strcmp(argv[arg_index], "curl") == 0)
  {
   arg_index++;

   bool verbose = false;
   if (arg_index < argc && std::strcmp(argv[arg_index], "--verbose") == 0)
   {
    verbose = true;
    arg_index++;
   }

   const char * url = nullptr;

   if (arg_index < argc)
   {
    url = argv[arg_index];
    arg_index++;
   }

   if (url && *url)
   {
    file.reset(new CURL_File(url, verbose));
    out << "OK\n";
   }
   else
    throw Runtime_Error("missing URL");
  }
#endif
#ifdef JOEDB_HAS_BROTLI
  else if (arg_index < argc + 1 && std::strcmp(argv[arg_index], "brotli") == 0)
  {
   arg_index++;
   const char * const file_name = argv[arg_index];
   arg_index++;
   file.reset(new Brotli_File(file_name));
  }
#endif
  else
  {
   if (arg_index < argc && std::strcmp(argv[arg_index], "file") == 0)
    arg_index++;

   joedb::Open_Mode open_mode = default_open_mode;

   if (arg_index < argc && !default_only)
   {
    for (size_t i = 0; i < open_mode_strings.size(); i++)
    {
     const std::string option = std::string("--") + open_mode_strings[i];
     if (option == argv[arg_index])
     {
      open_mode = Open_Mode(i);
      arg_index++;
     }
    }
   }

   const char *file_name = nullptr;
   if (arg_index < argc)
   {
    file_name = argv[arg_index];
    arg_index++;
   }

   out << "Opening local file (open_mode = ";
   out << open_mode_strings[size_t(open_mode)] << ") ... ";
   out.flush();

   if (file_name && *file_name)
   {
    file.reset(new File(file_name, open_mode));
    out << "OK\n";
   }
   else
    throw Runtime_Error("missing file name");
  }

  return *file;
 }
}
