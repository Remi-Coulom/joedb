#include "joedb/compiler/generator/File_Database_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 File_Database_h::File_Database_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "File_Database.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void File_Database_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard(out, "File_Database", options.get_name_space());

  out << R"RRR(
#include "Writable_Database.h"
#include "joedb/journal/File.h"

)RRR";


  namespace_open(out, options.get_name_space());

  out << R"RRR(
 namespace detail
 {
  class File_Database_Data
  {
   protected:
    joedb::File file;

    File_Database_Data(const char *file_name, joedb::Open_Mode mode):
     file(file_name, mode)
    {
    }
  };
 }

 /// Shortcut to directly build a @ref Writable_Database from a file name
 class File_Database:
  protected detail::File_Database_Data,
  public Writable_Database
 {
  public:
   File_Database
   (
    const char *file_name,
    joedb::Open_Mode mode = joedb::Open_Mode::write_existing_or_create_new,
    joedb::Readonly_Journal::Check check = joedb::Readonly_Journal::Check::all,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    File_Database_Data(file_name, mode),
    Writable_Database(file, check, commit_level)
   {
   }

   File_Database
   (
    const std::string &file_name,
    joedb::Open_Mode mode = joedb::Open_Mode::write_existing_or_create_new,
    joedb::Readonly_Journal::Check check = joedb::Readonly_Journal::Check::all,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    File_Database(file_name.c_str(), mode, check, commit_level)
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
