#include "joedb/compiler/generator/Generic_File_Database_cpp.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/io/type_io.h"
#include "joedb/io/write_value.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Generic_File_Database_cpp::Generic_File_Database_cpp
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Generic_File_Database.cpp", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File_Database_cpp::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const auto &db = options.get_db();
  const auto &tables = db.get_tables();

  out << "#include \"Generic_File_Database.h\"\n";
  out << "#include \"joedb/Writable.h\"\n";
  out << "#include \"joedb/journal/Readonly_Memory_File.h\"\n";
  out << '\n';
  out << "#include <ctime>\n\n";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::write_comment(const std::string &comment)
  ////////////////////////////////////////////////////////////////////////////
  {
   journal.comment(comment);
  }

  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::write_timestamp()
  ////////////////////////////////////////////////////////////////////////////
  {
   journal.timestamp(std::time(nullptr));
  }

  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::write_timestamp(int64_t timestamp)
  ////////////////////////////////////////////////////////////////////////////
  {
   journal.timestamp(timestamp);
  }

  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::write_valid_data()
  ////////////////////////////////////////////////////////////////////////////
  {
   journal.valid_data();
  }

  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::play_journal()
  ////////////////////////////////////////////////////////////////////////////
  {
   max_record_id = size_t(journal.get_checkpoint_position());
   ready_to_write = false;
   journal.play_until_checkpoint(*this);
   ready_to_write = true;
   max_record_id = 0;
  }

  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::auto_upgrade()
  ////////////////////////////////////////////////////////////////////////////
  {
   const size_t file_schema_size = size_t(schema_file.get_size());

   if (file_schema_size < schema_string_size)
   {
    journal.comment("Automatic schema upgrade");

    joedb::Readonly_Memory_File schema_file(schema_string, schema_string_size);
    joedb::Readonly_Journal schema_journal(schema_file);

    schema_journal.set_position(int64_t(file_schema_size));
    schema_journal.play_until(journal, schema_string_size);

    schema_journal.set_position(int64_t(file_schema_size));
    upgrading_schema = true;
    schema_journal.play_until(*this, schema_string_size);
    upgrading_schema = false;

    journal.valid_data();
   }
  }

  ////////////////////////////////////////////////////////////////////////////
  Generic_File_Database::Generic_File_Database
  ////////////////////////////////////////////////////////////////////////////
  (
   joedb::Generic_File &file,
   bool perform_initialization,
   joedb::Readonly_Journal::Check check,
   joedb::Commit_Level commit_level
  ):
   journal(file, check, commit_level)
  {
   journal.rewind();

   if (perform_initialization)
    initialize();
  }

  ////////////////////////////////////////////////////////////////////////////
  Generic_File_Database::Generic_File_Database
  ////////////////////////////////////////////////////////////////////////////
  (
   joedb::Generic_File &file,
   joedb::Readonly_Journal::Check check,
   joedb::Commit_Level commit_level
  ):
   Generic_File_Database(file, true, check, commit_level)
  {
  }

  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::check_single_row()
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
  void Generic_File_Database::create_table(const std::string &name)
  ////////////////////////////////////////////////////////////////////////////
  {
   Database::create_table(name);

   if (upgrading_schema)
   {
 )RRR";

   for (const auto &[tid, tname]: tables)
   {
    if (options.get_table_options(tid).single_row)
    {
     out << "   if (current_table_id == Table_Id{" << tid << "})\n";
     out << "    new_" << tname << "();\n";
    }
   }

   out << "  }\n }\n";
  }

  if (db_has_values())
  {
   out << R"RRR(
  ////////////////////////////////////////////////////////////////////////////
  void Generic_File_Database::add_field
  ////////////////////////////////////////////////////////////////////////////
  (
   Table_Id table_id,
   const std::string &name,
   joedb::Type type
  )
  {
   Database::add_field(table_id, name, type);
 )RRR";

   for (const auto &[tid, tname]: tables)
   {
    if (db.get_freedom(tid).size() > 0)
    {
     const Record_Id record_id{db.get_freedom(tid).get_first_used() - 1};

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
    out << "  /////////////////////////////////////////////////////////////////////////////\n";
    out << "  void Generic_File_Database::clear_" << tname << "_table()\n";
    out << "  /////////////////////////////////////////////////////////////////////////////\n";
    out << "  {\n";
    out << "   while (!get_" << tname << "_table().is_empty())\n";
    out << "    delete_" << tname << "(get_" << tname << "_table().last());\n";
    out << "  }\n";
   }
  }

  namespace_close(out, options.get_name_space());
 }
}
