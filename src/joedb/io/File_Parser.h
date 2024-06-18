#ifndef joedb_File_Parser_declared
#define joedb_File_Parser_declared

#include "joedb/journal/Generic_File.h"

#ifdef JOEDB_HAS_SSH
#include "joedb/ssh/Session.h"
#include "joedb/ssh/SFTP.h"
#endif

#include <memory>
#include <iosfwd>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class File_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const Open_Mode default_open_mode;
   const bool default_only;

#ifdef JOEDB_HAS_SSH
   std::unique_ptr<ssh::Session> ssh_session;
   std::unique_ptr<ssh::SFTP> sftp;
#endif
   std::unique_ptr<Generic_File> file;

  public:
   File_Parser
   (
    Open_Mode default_open_mode = Open_Mode::write_existing_or_create_new,
    bool default_only = false
   ):
    default_open_mode(default_open_mode),
    default_only(default_only)
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
