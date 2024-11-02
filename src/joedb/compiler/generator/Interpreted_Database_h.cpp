#include "joedb/compiler/generator/Interpreted_Database_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Interpreted_Database_h::Interpreted_Database_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Interpreted_Database.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreted_Database_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard(out, "Interpreted_Database", options.get_name_space());

  out << R"RRR(
#include "Generic_File_Database.h"
#include "joedb/journal/Interpreted_File.h"

)RRR";


  namespace_open(out, options.get_name_space());

  out << R"RRR(
 class Interpreted_Database_Parent
 {
  public:
   joedb::Interpreted_File file;

   Interpreted_Database_Parent(const char *file_name):
    file(file_name)
   {
   }
 };

 class Interpreted_Database:
  public Interpreted_Database_Parent,
  public Generic_File_Database
 {
  public:
   Interpreted_Database(const char *file_name):
    Interpreted_Database_Parent(file_name),
    Generic_File_Database(file)
   {
   }

   Interpreted_Database(const std::string &file_name):
    Interpreted_Database(file_name.c_str())
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";  
 }
}
