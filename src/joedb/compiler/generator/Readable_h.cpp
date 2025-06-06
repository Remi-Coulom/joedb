#include "joedb/compiler/generator/Readable_h.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/ui/type_io.h"

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
#include "Database_Writable.h"

#include "joedb/interpreted/Database_Schema.h"
#include "joedb/journal/Readonly_Memory_File.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 /// Implement the @ref joedb::Readable interface for a compiled database
 ///
 /// This allows using a compiled database with tools such as the
 /// command interpreter.
 class Readable: public joedb::Database_Schema
 {
  private:
   const Database &db;

  public:
   Readable(const Database &db): db(db)
   {
    joedb::Readonly_Memory_File file(detail::schema_string, detail::schema_string_size);
    joedb::Readonly_Journal journal(file);
    journal.replay_log(*this);
   }

   const joedb::Freedom_Keeper &get_freedom
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
)RRR";


   for (int type_index = 1; type_index < int(Type::type_ids); type_index++)
   {
    const auto type_id = Type::Type_Id(type_index);

    out << "\n  ";
    out << "const " << get_storage_type_string(type_id) << '&';
    out << " get_" << get_type_string(type_id);

    out << R"RRR(
  (
   Table_Id table_id,
   Record_Id record_id,
   Field_Id field_id
  ) const override
  {)RRR";

   for (const auto &[tid, tname]: tables)
   {
    bool has_typed_field = false;

    for (const auto &[fid, fname]: db.get_fields(tid))
    {
     const Type &type = db.get_field_type(tid, fid);
     if (type.get_type_id() == type_id)
     {
      has_typed_field = true;
      break;
     }
    }

    if (has_typed_field)
    {
     out << "\n   if (table_id == Table_Id{" << to_underlying(tid) << "})\n";
     out << "   {\n";

     for (const auto &[fid, fname]: db.get_fields(tid))
     {
      const Type &type = db.get_field_type(tid, fid);

      if (type.get_type_id() == type_id)
      {
       out << "    if (field_id == Field_Id(" << fid << "))\n";
       out << "    {\n";
       out << "     return ";

       if (type_id == joedb::Type::Type_Id::reference)
        out << "*reinterpret_cast<const joedb::Record_Id *>(&";

       out << "db.storage_of_" << tname << ".field_value_of_" << fname << "[to_underlying(record_id)]";

       if (type_id == joedb::Type::Type_Id::reference)
        out << ")";

       out << ";\n";
       out << "    }\n";
      }
     }

     out << "   }\n";
    }
   }

   out << R"RRR(
   throw joedb::Exception("unknown field");
  }
)RRR";
   }

  out << " };\n";

  for (const auto &[tid, tname]: tables)
  {
   out << "\n namespace interpreted_" << tname << '\n';
   out << " {\n";
   out << "  constexpr static Table_Id table_id = Table_Id{" << int(tid) << "};\n";
   for (const auto &[fid, fname]: db.get_fields(tid))
    out << "  constexpr static Field_Id " << fname << "_field_id = Field_Id{" << int(fid) << "};\n";
   out << " }\n";
  }

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
