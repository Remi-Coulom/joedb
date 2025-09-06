#include "joedb/compiler/generator/print_table_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 print_table_h::print_table_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "print_table.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void print_table_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard_open(out, "print_table", options.get_name_space());

  out << R"RRR(
#include "Readable.h"
#include "joedb/ui/Readable_Command_Processor.h"

)RRR";

  namespace_open(out, options.get_name_space());

  for (const auto &[tid, tname]: options.get_db().get_tables())
  {
  out << R"RRR(
 inline void print_)RRR" << tname << R"RRR(_table
 (
  std::ostream &out,
  const Database &db,
  size_t max_column_width = 0,
  Record_Id start = Record_Id::null,
  size_t length = 0
 )
 {
  Readable readable(db);
  joedb::Readable_Command_Processor processor(readable);
  processor.print_table
  (
   out,
   )RRR" << tname << R"RRR(_table::id,
   max_column_width,
   start,
   length
  );
 }
)RRR";
  }

  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
