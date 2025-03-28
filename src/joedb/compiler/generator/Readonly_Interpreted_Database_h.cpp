#include "joedb/compiler/generator/Readonly_Interpreted_Database_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Readonly_Interpreted_Database_h::Readonly_Interpreted_Database_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Readonly_Interpreted_Database.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Readonly_Interpreted_Database_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard(out, "Readonly_Interpreted_Database", options.get_name_space());

  out << R"RRR(
#include "Readonly_Database.h"
#include "joedb/journal/Readonly_Interpreted_File.h"

)RRR";


  namespace_open(out, options.get_name_space());

  out << R"RRR(
 class Readonly_Interpreted_Database:
  private joedb::Readonly_Interpreted_File,
  public Readonly_Database
 {
  public:
   Readonly_Interpreted_Database(const char *file_name):
    joedb::Readonly_Interpreted_File(file_name),
    Readonly_Database(*static_cast<joedb::Readonly_Interpreted_File *>(this))
   {
   }

   Readonly_Interpreted_Database(const std::string &file_name):
    Readonly_Interpreted_Database(file_name.c_str())
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";  
 }
}
