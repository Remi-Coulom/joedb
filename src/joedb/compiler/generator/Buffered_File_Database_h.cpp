#include "joedb/compiler/generator/Buffered_File_Database_h.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/io/type_io.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Buffered_File_Database_h::Buffered_File_Database_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Buffered_File_Database.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Buffered_File_Database_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const Database &db = options.get_db();
  auto tables = db.get_tables();

  namespace_include_guard(out, "Buffered_File_Database", options.get_name_space());

  out << R"RRR(
#include "Database.h"
#include "joedb/Span.h"
#include "joedb/Multiplexer.h"

)RRR";

  namespace_open(out, options.get_name_space());

  //
  // Buffered_File_Database
  //
  out << R"RRR(
 class Client_Data;
 class Client;

 class Buffered_File_Database: public Database, public joedb::Blob_Reader
 {
  friend class Client_Data;
  friend class Client;

  private:
   joedb::Writable_Journal journal;
   bool ready_to_write;

   void play_journal();
   void auto_upgrade();
   void check_single_row();

   void initialize()
   {
    play_journal();
    check_schema();
    auto_upgrade();
    check_single_row();
    default_checkpoint();
   }
)RRR";


  if (!options.get_custom_names().empty())
  {
   out << R"RRR(
   void custom(const std::string &name) final
   {
    Database::custom(name);

    if (upgrading_schema)
    {
)RRR";
   for (const auto &name: options.get_custom_names())
   {
    out << "     if (name == \"" << name << "\")\n";
    out << "      " << name << "(*this);\n";
   }
   out << "    }\n";
   out << "   }\n";

   out << '\n';
   out << "  public:\n";
   for (const auto &name: options.get_custom_names())
    out << "   static void " << name << "(Buffered_File_Database &db);\n";
   out << "\n  private:";
  }

  if (db_has_values())
  {
   out << R"RRR(
   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    joedb::Type type
   ) override;
)RRR";
  }

  if (options.has_single_row())
  {
   out <<"\n   void create_table(const std::string &name) override;\n";
  }

  out << R"RRR(
   Buffered_File_Database
   (
    joedb::Buffered_File &file,
    bool perform_initialization,
    joedb::Readonly_Journal::Check check,
    joedb::Commit_Level commit_level
   );

  public:
   Buffered_File_Database
   (
    joedb::Buffered_File &file,
    joedb::Readonly_Journal::Check check = joedb::Readonly_Journal::Check::all,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   );

   std::string read_blob_data(joedb::Blob blob) final
   {
    return journal.read_blob_data(blob);
   }

   joedb::Blob write_blob_data(const std::string &data) final
   {
    return journal.write_blob_data(data);
   }

   int64_t ahead_of_checkpoint() const
   {
    return journal.ahead_of_checkpoint();
   }

   void checkpoint_no_commit()
   {
    journal.checkpoint(joedb::Commit_Level::no_commit);
   }

   void checkpoint_half_commit()
   {
    journal.checkpoint(joedb::Commit_Level::half_commit);
   }

   void checkpoint_full_commit()
   {
    journal.checkpoint(joedb::Commit_Level::full_commit);
   }

   void checkpoint()
   {
    journal.default_checkpoint();
   }

   void checkpoint(joedb::Commit_Level commit_level) final
   {
    journal.checkpoint(commit_level);
   }

   void write_comment(const std::string &comment);
   void write_timestamp();
   void write_timestamp(int64_t timestamp);
   void write_valid_data();
)RRR";

  for (const auto &[tid, tname]: tables)
  {
   out << '\n';
   const bool single_row = options.get_table_options(tid).single_row;

   //
   // Erase all elements of the table
   //
   if (!single_row)
   {
    out << "   void clear_" << tname << "_table();\n";
    out << '\n';
   }

   if (single_row)
    out << "  private:\n";

   //
   // Uninitialized new
   //
   out << "   id_of_" << tname << " new_" << tname << "()\n";
   out << "   {\n";

   out << "    id_of_" << tname << " result(Record_Id(storage_of_" << tname << ".freedom_keeper.get_free_record() - 1));\n";
   out << "    storage_of_" << tname << ".resize(storage_of_" << tname << ".freedom_keeper.size());\n";
   out << "    internal_insert_" << tname << "(result.get_record_id());\n\n";
   out << "    journal.insert_into(Table_Id(" << tid << "), result.get_record_id());\n";
   out << "    return result;\n";
   out << "   }\n";
   out << '\n';

   //
   // new uninitialized vector
   //
   if (!single_row)
   {
    out << "   id_of_" << tname << " new_vector_of_" << tname << "(size_t size)\n";
    out << "   {\n";
    out << "    id_of_" << tname << " result(Record_Id(storage_of_" << tname;
    out << ".size() + 1));\n";
    out << "    storage_of_" << tname << ".resize(storage_of_";
    out << tname << ".size() + size);\n";
    out << "    internal_vector_insert_" << tname << "(result.get_record_id(), size);\n";
    out << "    journal.insert_vector(Table_Id(" << tid;
    out << "), result.get_record_id(), size);\n";
    out << "    return result;\n";
    out << "   }\n";
    out << '\n';
   }

   //
   // new with all fields
   //
   if (!db.get_fields(tid).empty())
   {
    out << "   id_of_" << tname << " new_" << tname << '\n';
    out << "   (\n    ";
    {
     bool first = true;

     for (const auto &[fid, fname]: db.get_fields(tid))
     {
      if (first)
       first = false;
      else
       out << ",\n    ";

      const Type &type = db.get_field_type(tid, fid);
      write_type(type, false, true);
      out << " field_value_of_" << fname;
     }

     out << '\n';
    }
    out << "   )\n";
    out << "   {\n";
    out << "    auto result = new_" << tname << "();\n";

    for (const auto &[fid, fname]: db.get_fields(tid))
     out << "    set_" << fname << "(result, field_value_of_" << fname << ");\n";

    out << "    return result;\n";
    out << "   }\n\n";
   }

   if (single_row)
    out << "  public:\n";

   //
   // Delete
   //
   if (!single_row)
   {
    out << "   void delete_" << tname << "(id_of_" << tname << " record)\n";
    out << "   {\n";
    out << "    internal_delete_" << tname << "(record.get_record_id());\n";
    out << "    journal.delete_from(Table_Id(" << tid << "), record.get_record_id());\n";
    out << "   }\n\n";

    out << "   void delete_vector_of_" << tname << "(id_of_" << tname << " v, size_t size)\n";
    out << "   {\n";
    out << "    for (size_t i = size; i > 0;)\n";
    out << "     delete_" << tname << "(v[--i]);\n";
    out << "   }\n\n";
   }

   //
   // Loop over fields
   //
   for (const auto &[fid, fname]: db.get_fields(tid))
   {
    const Type &type = db.get_field_type(tid, fid);

    //
    // Setter
    //
    out << "   void set_" << fname;
    out << "(id_of_" << tname << " record, ";
    write_type(type, false, true);
    out << " field_value_of_" << fname << ")\n";
    out << "   {\n";
    out << "    internal_update_" << tname << "__" << fname;

    out <<  "(record.get_record_id(), ";
    out << "field_value_of_" << fname << ");\n";
    out << "    journal.update_";
    out << get_type_string(type);
    out << "(Table_Id(" << tid << "), record.get_record_id(), Field_Id(" << fid << "), ";
    out << "field_value_of_" << fname;
    if (type.get_type_id() == Type::Type_Id::reference)
     out << ".get_record_id()";
    out << ");\n";

    out << "   }\n\n";

    //
    // Vector update
    // Note: write even if exception, to keep memory and file in sync
    //
    out << "   template<typename F> void update_vector_of_" << fname;
    out << "(id_of_" << tname << " record, size_t size, F f)\n";
    out << "   {\n";
    out << "    std::exception_ptr exception;\n";
    out << "    joedb::Span<";
    write_type(type, false, false);
    out << "> span(&storage_of_" << tname;
    out << ".field_value_of_" << fname << "[record.get_id() - 1], size);\n";
    out << "    try {f(span);}\n";
    out << "    catch (...) {exception = std::current_exception();}\n";
    out << "    internal_update_vector_" << tname << "__" << fname << "(record.get_record_id(), size, span.begin());\n";
    out << "    journal.update_vector_" << get_type_string(type) << "(Table_Id(" << tid << "), record.get_record_id(), Field_Id(" << fid << "), size, ";

    if (type.get_type_id() == Type::Type_Id::reference)
     out << "reinterpret_cast<Record_Id *>";

    out << "(span.begin()));\n";
    out << "    if (exception)\n";
    out << "     std::rethrow_exception(exception);\n";
    out << "   }\n\n";
   }
  }

  out << R"RRR(   joedb::Multiplexer get_multiplexer()
   {
    return joedb::Multiplexer{*this, journal};
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
