#include "joedb/compiler/generator/Readable_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Readable_h::Readable_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Readable.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Readable_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const Database_Schema &db = options.get_db();
  auto tables = db.get_tables(); 
  
  namespace_include_guard(out, "Readable", options.get_name_space());

  out << R"RRR(
#include "Database.h"

#include "joedb/interpreter/Database_Schema.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 class Readable: public joedb::Database_Schema
 {
  private:
   const Database &db;

  public:
   Readable(const Database &db): db(db)
   {
    // TODO: load schema from schema string
   }

   const joedb::Compact_Freedom_Keeper &get_freedom
   (
    Table_Id table_id
   ) const override
   {
)RRR";

   for (const auto &[tid, tname]: tables)
   {
    out << "    if (table_id == Table_Id{" << to_underlying(tid) << "})\n";
    out << "     return db.storage_of_" << tname << ".freedom_keeper;\n";
   }

   out << R"RRR(
    throw joedb::Exception("unknown table_id");
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());

  out << "\n#endif\n";  
 }
}
