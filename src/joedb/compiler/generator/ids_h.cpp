#include "joedb/compiler/generator/ids_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 ids_h::ids_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options,
  const Compiler_Options *parent_options
 ):
  Generator(".", "ids.h", options),
  parent_options(parent_options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void ids_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const Database_Schema &db = options.get_db();
  auto tables = db.get_tables();

  namespace_include_guard(out, "ids", options.get_name_space());

  if (parent_options)
   out << R"RRR(
#include "../../ids.h"

)RRR";
  else
   out << R"RRR(
#include "joedb/index_types.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 using joedb::Record_Id;
 using joedb::Table_Id;
 using joedb::Field_Id;
)RRR";

  for (const auto &[tid, tname]: tables)
  {
   out << '\n';
   if (parent_options && parent_options->has_table(tname))
   {
    out << " using id_of_" << tname << " = ";
    namespace_write(out, parent_options->name_space);
    out << "::id_of_" << tname << ";\n";
   }
   else
   {
    out << " /// Strongly-typed wrapper around an integer representing a row of the " << tname << " table\n";
    out << " class id_of_" << tname << "\n {\n";
    out << "  private:\n";
    out << "   Record_Id id;\n";
    out << "\n  public:\n";
    out << "   constexpr explicit id_of_" << tname << "(joedb::index_t id): id(Record_Id(id)) {}\n";
    out << "   constexpr explicit id_of_" << tname << "(Record_Id id): id(id) {}\n";
    out << "   constexpr id_of_" << tname << "(): id(joedb::Record_Id::null) {}\n";
    out << "   constexpr bool is_null() const {return id.is_null();}\n";
    out << "   constexpr bool is_not_null() const {return id.is_not_null();}\n";
    out << "   constexpr auto get_id() const {return to_underlying(id);}\n";
    out << "   constexpr Record_Id get_record_id() const {return id;}\n";
    out << "   constexpr bool operator==(id_of_" << tname << " x) const {return id == x.id;}\n";
    out << "   constexpr bool operator!=(id_of_" << tname << " x) const {return id != x.id;}\n";
    out << "   constexpr bool operator<(id_of_" << tname << " x) const {return id < x.id;}\n";
    out << "   constexpr bool operator>(id_of_" << tname << " x) const {return id > x.id;}\n";
    out << "   constexpr bool operator<=(id_of_" << tname << " x) const {return id <= x.id;}\n";
    out << "   constexpr bool operator>=(id_of_" << tname << " x) const {return id >= x.id;}\n";
    out << "   constexpr id_of_" << tname << " operator[](size_t i) const {return id_of_" << tname << "(id + i);}\n";
    out << " };\n";
   }
  }

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
