#include "joedb/compiler/generator/Readonly_Interpreted_File_Database_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Readonly_Interpreted_File_Database_h::Readonly_Interpreted_File_Database_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Readonly_Interpreted_File_Database.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Readonly_Interpreted_File_Database_h::write(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard_open(out, "Readonly_Interpreted_File_Database", options.get_name_space());

  out << R"RRR(
#include "Readonly_Database.h"
#include "joedb/journal/Interpreted_File.h"

)RRR";


  namespace_open(out, options.get_name_space());

  out << R"RRR(
 /// @ref Readonly_Database for a .joedbi text file
 class Readonly_Interpreted_File_Database: public Readonly_Database
 {
  public:
   Readonly_Interpreted_File_Database(const char *file_name):
    Readonly_Database(joedb::Interpreted_File(file_name, joedb::Open_Mode::read_existing))
   {
   }

   Readonly_Interpreted_File_Database(const std::string &file_name):
    Readonly_Interpreted_File_Database(file_name.c_str())
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
