#include "joedb/compiler/generator/Interpreted_File_Database_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File_Database_h::Interpreted_File_Database_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Interpreted_File_Database.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreted_File_Database_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard_open(out, "Interpreted_File_Database", options.get_name_space());

  out << R"RRR(
#include "Writable_Database.h"
#include "joedb/journal/Interpreted_File.h"

)RRR";


  namespace_open(out, options.get_name_space());

  out << R"RRR(
 /// Open a .joedbi text file for reading or writing
 class Interpreted_File_Database:
  private joedb::Interpreted_File,
  public Writable_Database
 {
  public:
   Interpreted_File_Database(const char *file_name):
    joedb::Interpreted_File(file_name, joedb::Open_Mode::write_existing_or_create_new),
    Writable_Database(*static_cast<joedb::Interpreted_File *>(this))
   {
   }

   Interpreted_File_Database(const std::string &file_name):
    Interpreted_File_Database(file_name.c_str())
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
