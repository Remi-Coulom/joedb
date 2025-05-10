#ifndef joedb_File_Parser_declared
#define joedb_File_Parser_declared

#include "joedb/journal/Buffered_File.h"
#include "joedb/ui/Arguments.h"

#ifdef JOEDB_HAS_SSH
#include "joedb/ssh/Session.h"
#include "joedb/ssh/SFTP.h"
#endif

#include <memory>
#include <iosfwd>
#include <optional>

namespace joedb
{
 /// Create an instance of a @ref Buffered_File by parsing command-line arguments
 /// @ingroup ui
 class File_Parser
 {
  private:
   const Open_Mode default_open_mode;
   const bool default_only;
   const bool include_shared;
   const bool include_server;

#ifdef JOEDB_HAS_SSH
   std::optional<ssh::Session> ssh_session;
   std::optional<ssh::SFTP> sftp;
#endif
   std::unique_ptr<Buffered_File> file;

  public:
   File_Parser
   (
    Open_Mode default_open_mode = Open_Mode::write_existing_or_create_new,
    bool default_only = false,
    bool include_shared = true,
    bool include_server = false
   ):
    default_open_mode(default_open_mode),
    default_only(default_only),
    include_shared(include_shared),
    include_server(include_server)
   {
   }

   Buffered_File *parse(std::ostream &out, Arguments &arguments);
   Buffered_File *get_file() const {return file.get();}
   void print_help(std::ostream &out) const;
 };
}

#endif
