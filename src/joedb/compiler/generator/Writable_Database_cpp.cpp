#include "joedb/compiler/generator/Writable_Database_cpp.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/ui/type_io.h"
#include "joedb/ui/write_value.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Writable_Database_cpp::Writable_Database_cpp
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Writable_Database.cpp", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database_cpp::write(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  const auto &db = options.get_db();
  const auto &tables = db.get_tables();

  out << "#include \"Writable_Database.h\"\n";
  out << "#include \"joedb/journal/Readonly_Memory_File.h\"\n";
  out << '\n';
  out << "#include <ctime>\n\n";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::write_comment(const std::string &comment)
 ////////////////////////////////////////////////////////////////////////////
 {
  journal.comment(comment);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::write_timestamp()
 ////////////////////////////////////////////////////////////////////////////
 {
  journal.timestamp(std::time(nullptr));
 }

 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::write_timestamp(int64_t timestamp)
 ////////////////////////////////////////////////////////////////////////////
 {
  journal.timestamp(timestamp);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::write_valid_data()
 ////////////////////////////////////////////////////////////////////////////
 {
  journal.valid_data();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::play_journal()
 ////////////////////////////////////////////////////////////////////////////
 {
  max_record_id = size_t(journal.get_checkpoint());
  ready_to_write = false;
  journal.play_until_checkpoint(*this);
  ready_to_write = true;
  max_record_id = 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::auto_upgrade()
 ////////////////////////////////////////////////////////////////////////////
 {
  const size_t file_schema_size = size_t(schema_file.get_size());

  if (file_schema_size < detail::schema_string_size)
  {
   journal.comment("Automatic schema upgrade");

   joedb::Readonly_Memory_File schema_file(detail::schema_string, detail::schema_string_size);
   joedb::Readonly_Journal schema_journal(schema_file);

   schema_journal.skip_directly_to(int64_t(file_schema_size));
   schema_journal.raw_play_until(journal, detail::schema_string_size);

   schema_journal.skip_directly_to(int64_t(file_schema_size));
   upgrading_schema = true;
   schema_journal.raw_play_until(*this, detail::schema_string_size);
   upgrading_schema = false;

   journal.valid_data();
   journal.soft_checkpoint();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::initialize()
 ////////////////////////////////////////////////////////////////////////////
 {
  play_journal();
  check_schema();
  auto_upgrade();
  check_single_row();
 }

 ////////////////////////////////////////////////////////////////////////////
 Writable_Database::Writable_Database
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Abstract_File &file,
  joedb::Recovery recovery,
  bool perform_initialization
 ):
  journal(joedb::Journal_Construction_Lock(file, recovery))
 {
  journal.rewind();

  if (perform_initialization)
   initialize();
 }

 ////////////////////////////////////////////////////////////////////////////
 Writable_Database::Writable_Database(joedb::Abstract_File &file):
 ////////////////////////////////////////////////////////////////////////////
  Writable_Database(file, joedb::Recovery::none, true)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 Writable_Database::Writable_Database
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Abstract_File &file,
  joedb::Recovery recovery
 ):
  Writable_Database(file, recovery, true)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::check_single_row()
 ////////////////////////////////////////////////////////////////////////////
 {
)RRR";

  for (const auto &[tid, tname]: tables)
  {
   if (options.get_table_options(tid).single_row)
   {
    out << "  {\n";
    out << "   const auto table = get_" << tname << "_table();\n";
    out << "   if (table.first() != the_" <<tname << "() || table.last() != the_" << tname << "())\n";
    out << "    throw joedb::Exception(\"Single-row constraint failure for table " << tname << "\");\n";
    out << "  }\n";
   }
  }
  out << " }\n";

  if (options.has_single_row())
  {
   out << R"RRR(
 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::create_table(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  Database_Writable::create_table(name);

  if (upgrading_schema)
  {
 )RRR";

   for (const auto &[tid, tname]: tables)
   {
    if (options.get_table_options(tid).single_row)
    {
     out << "  if (current_table_id == Table_Id{" << tid << "})\n";
     out << "   new_" << tname << "();\n";
    }
   }

   out << "   }\n  }\n";
  }

  if (db_has_values())
  {
   out << R"RRR(
 ////////////////////////////////////////////////////////////////////////////
 void Writable_Database::add_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name,
  joedb::Type type
 )
 {
  Database_Writable::add_field(table_id, name, type);
 )RRR";

   for (const auto &[tid, tname]: tables)
   {
    if (db.get_freedom(tid).get_used_count() > Record_Id{0})
    {
     const Record_Id record_id{db.get_freedom(tid).get_first_used()};

     out << "\n  if (table_id == Table_Id{" << tid << "})\n";
     out << "  {\n";
     out << "   const auto field_id = ++storage_of_" << tname << ".current_field_id;\n";
     out << "   if (upgrading_schema)\n";
     out << "   {\n";

     for (const auto &[fid, fname]: db.get_fields(tid))
     {
      out << "    if (field_id == Field_Id{" << fid  << "})\n";
      out << "    {\n";
      out << "     for (const auto record: get_" << tname << "_table())\n";
      out << "      set_" << fname << "(record, ";

      const auto &type = db.get_field_type(tid, fid);
      const bool reference = type.get_type_id() == joedb::Type::Type_Id::reference;

      if (reference)
      {
       const auto it = tables.find(type.get_table_id());
       if (it != tables.end())
        out << "id_of_" << it->second;
       out << "(";
      }

      joedb::write_value(out, db, tid, record_id, fid);

      if (reference)
       out << ")";

      out <<");\n";
      out << "    }\n";
     }

     out << "   }\n";
     out << "  }\n";
    }
   }
   out << " }\n";
  }

  for (const auto &[tid, tname]: tables)
  {
   const bool single_row = options.get_table_options(tid).single_row;

   if (!single_row)
   {
    out << '\n';
    out << " /////////////////////////////////////////////////////////////////////////////\n";
    out << " void Writable_Database::clear_" << tname << "_table()\n";
    out << " /////////////////////////////////////////////////////////////////////////////\n";
    out << " {\n";
    out << "  while (!get_" << tname << "_table().is_empty())\n";
    out << "   delete_" << tname << "(get_" << tname << "_table().last());\n";
    out << " }\n";
   }
  }

  namespace_close(out, options.get_name_space());
  out.flush();
 }
}
