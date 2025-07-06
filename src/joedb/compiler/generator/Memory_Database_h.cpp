#include "joedb/compiler/generator/Memory_Database_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Memory_Database_h::Memory_Database_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Memory_Database.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Memory_Database_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard(out, "Memory_Database", options.get_name_space());

  out << R"RRR(
#include "Writable_Database.h"
#include "joedb/journal/Memory_File.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 /// Shortcut to directly build a @ref Writable_Database with a Memory_File
 class Memory_Database: public joedb::Memory_File, public Writable_Database
 {
  public:
   Memory_Database(): Writable_Database(*this, joedb::Recovery::none)
   {
   }

   void pull()
   {
    journal.pull();
    play_journal();
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
