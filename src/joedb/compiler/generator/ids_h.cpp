#include "joedb/compiler/generator/ids_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 ids_h::ids_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "ids.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void ids_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const Database_Schema &db = options.get_db();
  auto tables = db.get_tables();

  namespace_include_guard(out, "ids", options.get_name_space());

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
   out << " /// Strongly-typed wrapper around an integer representing a row of the " << tname << " table\n";
   out << " class id_of_" << tname << "\n {\n";
   out << "  private:\n";
   out << "   Record_Id id;\n";
   out << "\n  public:\n";
   out << "   constexpr explicit id_of_" << tname << "(size_t id): id(Record_Id(id)) {}\n";
   out << "   constexpr explicit id_of_" << tname << "(Record_Id id): id(id) {}\n";
   out << "   constexpr id_of_" << tname << "(): id(Record_Id(0)) {}\n";
   out << "   constexpr bool is_null() const {return id == Record_Id(0);}\n";
   out << "   constexpr bool is_not_null() const {return id != Record_Id(0);}\n";
   out << "   constexpr size_t get_id() const {return size_t(id);}\n";
   out << "   constexpr Record_Id get_record_id() const {return id;}\n";
   out << "   constexpr bool operator==(id_of_" << tname << " x) const {return id == x.id;}\n";
   out << "   constexpr bool operator!=(id_of_" << tname << " x) const {return id != x.id;}\n";
   out << "   constexpr bool operator<(id_of_" << tname << " x) const {return id < x.id;}\n";
   out << "   constexpr bool operator>(id_of_" << tname << " x) const {return id > x.id;}\n";
   out << "   constexpr bool operator<=(id_of_" << tname << " x) const {return id <= x.id;}\n";
   out << "   constexpr bool operator>=(id_of_" << tname << " x) const {return id >= x.id;}\n";
   out << "   constexpr id_of_" << tname << " operator[](size_t i) const {return id_of_" << tname << "(id + i);}\n\n";

   out << "   constexpr static Table_Id table_id = Table_Id{" << int(tid) << "};\n";
   for (const auto &[fid, fname]: db.get_fields(tid))
    out << "   constexpr static Field_Id id_of_" << fname << " = Field_Id{" << int(fid) << "};\n";
   out << " };\n";
  }

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
