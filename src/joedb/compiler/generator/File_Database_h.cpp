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
 void File_Database_h::write(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard_open(out, "File_Database", options.get_name_space());

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
    joedb::Recovery recovery = joedb::Recovery::none
   ):
    File_Database_Data(file_name, mode),
    Writable_Database(file, recovery)
   {
   }

   File_Database
   (
    const std::string &file_name,
    joedb::Open_Mode mode = joedb::Open_Mode::write_existing_or_create_new,
    joedb::Recovery recovery = joedb::Recovery::none
   ):
    File_Database(file_name.c_str(), mode, recovery)
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
