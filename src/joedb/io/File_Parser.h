#ifndef joedb_File_Parser_declared
#define joedb_File_Parser_declared

#include "joedb/journal/Generic_File.h"
#include "joedb/journal/File.h"

#ifdef JOEDB_HAS_SSH
#include "joedb/ssh/Session.h"
#include "joedb/ssh/SFTP.h"
#endif

#include <memory>
#include <iosfwd>
#include <optional>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class File_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const Open_Mode default_open_mode;
   const bool default_only;
   const bool include_shared;

#ifdef JOEDB_HAS_SSH
   std::optional<ssh::Session> ssh_session;
   std::optional<ssh::SFTP> sftp;
#endif
   std::unique_ptr<Generic_File> file;

  public:
   File_Parser
   (
    Open_Mode default_open_mode = Open_Mode::write_existing_or_create_new,
    bool default_only = false,
    bool include_shared = true
   ):
    default_open_mode(default_open_mode),
    default_only(default_only),
#ifdef JOEDB_FILE_IS_LOCKABLE
    include_shared(include_shared)
#else
    include_shared(false)
#endif
   {
   }

   Generic_File &parse
   (
    std::ostream &out,
    int argc,
    char **argv,
    int &arg_index
   );

   Generic_File &get_file() const {return *file;}

   void print_help(std::ostream &out) const;
 };
}

#endif
