#include "joedb/ui/File_Parser.h"
#include "joedb/ui/open_mode_strings.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Interpreted_File.h"

#ifdef JOEDB_HAS_SSH
#include "joedb/journal/SFTP_File.h"
#endif

#ifdef JOEDB_HAS_CURL
#include "joedb/journal/CURL_File.h"
#endif

#ifdef JOEDB_HAS_BROTLI
#include "joedb/journal/Brotli_File.h"
#include "joedb/journal/Readonly_Brotli_File.h"
#endif

#include <cstring>
#include <iostream>

namespace joedb
{
 static constexpr size_t open_modes = open_mode_strings.size() -
  (File::lockable ? 0 : 2);

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

   for (size_t i = 0; i < open_modes; i++)
   {
    const Open_Mode mode = Open_Mode(i);
    if (!include_shared && mode == Open_Mode::shared_write)
     continue;
    out << "  " << open_mode_strings[i];
    if (mode == default_open_mode)
     out << '*';
    out << '\n';
   }

   out << " memory\n";
  }

  out << " interpreted [--read] <file_name>\n";

#ifdef JOEDB_HAS_SSH
  out << " sftp [--port p] [--verbosity v] <user> <host> <file_name>\n";
#endif

#ifdef JOEDB_HAS_CURL
  out << " curl [--verbose] <URL>\n";
#endif

#ifdef JOEDB_HAS_BROTLI
  out << " brotli ";
  if (!default_only)
   out << "[--read] ";
  out << "<file_name>\n";
#endif

#if defined(JOEDB_HAS_ASIO) || defined(JOEDB_HAS_SSH) || defined(JOEDB_HAS_WEBSOCKETS)
  if (include_server)
   out << " server (client must use a connection to a server)\n";
#endif
 }

 ////////////////////////////////////////////////////////////////////////////
 Buffered_File *File_Parser::parse(std::ostream &out, Arguments &arguments)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (arguments.peek("memory"))
   file.reset(new Memory_File());
  else if (arguments.peek("server"))
   file.reset();
  else if (arguments.peek("interpreted"))
  {
   bool readonly = false;

   if (default_only)
    readonly = default_open_mode == Open_Mode::read_existing;
   else if (arguments.peek("--read"))
    readonly = true;

   const std::string_view file_name = arguments.get_next();
   if (arguments.missing())
    return nullptr;

   out << "Opening interpreted file... ";
   out.flush();

   if (readonly)
    file.reset(new Readonly_Interpreted_File(file_name.data()));
   else
    file.reset(new Interpreted_File(file_name.data()));

   out << "OK\n";
  }
#ifdef JOEDB_HAS_SSH
  else if (arguments.peek("sftp"))
  {
   std::string_view port_string;
   if (arguments.peek("--port"))
    port_string = arguments.get_next();

   std::string_view verbosity_string;
   if (arguments.peek("--verbosity"))
    verbosity_string = arguments.get_next();

   const std::string_view user = arguments.get_next();
   const std::string_view host = arguments.get_next();
   const std::string_view file_name = arguments.get_next();

   if (arguments.missing())
    return nullptr;

   unsigned port = 22;
   if (port_string.data())
    port = uint16_t(std::atoi(port_string.data()));

   int verbosity = 0;
   if (verbosity_string.data())
    verbosity = std::atoi(verbosity_string.data());

   out << "Creating ssh Session... ";
   out.flush();
   ssh_session.emplace(user.data(), host.data(), port, verbosity);

   out << "OK\nInitializing sftp... ";
   out.flush();
   sftp.emplace(*ssh_session);

   out << "OK\nOpening file... ";
   out.flush();
   file.reset(new SFTP_File(*sftp, file_name.data()));

   out << "OK\n";
  }
#endif
#ifdef JOEDB_HAS_CURL
  else if (arguments.peek("curl"))
  {
   const bool verbose = arguments.peek("--verbose");
   const std::string_view url = arguments.get_next();

   if (arguments.missing())
    return nullptr;

   file.reset(new CURL_File(url.data(), verbose));
  }
#endif
#ifdef JOEDB_HAS_BROTLI
  else if (arguments.peek("brotli"))
  {
   bool readonly = false;

   if (default_only)
    readonly = default_open_mode == Open_Mode::read_existing;
   else if (arguments.peek("--read"))
    readonly = true;

   const std::string_view file_name = arguments.get_next();

   if (arguments.missing())
    return nullptr;

   out << "Opening brotli file... ";
   out.flush();

   if (readonly)
    file.reset(new Readonly_Brotli_File(file_name.data()));
   else
    file.reset(new Brotli_File(file_name.data()));

   out << "OK\n";
  }
#endif
  else
  {
   arguments.peek("file");

   Open_Mode open_mode = default_open_mode;

   if (!default_only)
   {
    for (size_t i = 0; i < open_modes; i++)
    {
     const Open_Mode mode = Open_Mode(i);
     if (!include_shared && mode == Open_Mode::shared_write)
      continue;
     const std::string option = std::string("--") + open_mode_strings[i];
     if (arguments.peek(option.data()))
      open_mode = mode;
    }
   }

   const std::string_view file_name = arguments.get_next();

   if (arguments.missing())
    return nullptr;

   out << "Opening local file (open_mode = ";
   out << open_mode_strings[size_t(open_mode)] << ") ... ";
   out.flush();
   file.reset(new File(file_name.data(), open_mode));
  }

  return file.get();
 }
}
