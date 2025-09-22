#include "joedb/compiler/generator/Database_Writable_h.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/ui/type_io.h"

#include <set>

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Database_Writable_h::Database_Writable_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Database_Writable.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_Writable_h::write(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  const Database_Schema &db = options.get_db();
  auto tables = db.get_tables();

  namespace_include_guard_open(out, "Database_Writable", options.get_name_space());

  out << R"RRR(
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/error/Out_Of_Date.h"
#include "Database.h"

#include <string>
#include <stdint.h>
#include <cstring>
#include <vector>

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 namespace detail
 {
  extern const char * schema_string;
  inline constexpr size_t schema_string_size = )RRR";

  out << options.schema_file.get_size() << ";\n }\n\n";

  for (const auto &[tid, tname]: tables)
   out << " class container_of_" << tname << ";\n";

  out << R"RRR(
 /// implement @ref joedb::Writable in a @ref Database
 class Database_Writable: public Database, public joedb::Writable
 {
  protected:
   joedb::index_t max_record_id;
   Table_Id current_table_id = Table_Id{0};
)RRR";

  //
  // delete_from writable function
  //
  out << '\n';
  out << "   void delete_from(Table_Id table_id, Record_Id record_id) override\n";
  out << "   {\n";
  {
   bool first = true;
   for (const auto &[tid, tname]: tables)
   {
    out << "    ";
    if (first)
     first = false;
    else
     out << "else ";

    out << "if (table_id == Table_Id(" << tid << "))\n";
    out << "     internal_delete_" << tname << "(record_id);\n";
   }
  }
  out << "   }\n";

  //
  // insert_into
  //
  out << '\n';
  out << "   void insert_into(Table_Id table_id, Record_Id record_id) override\n";
  out << "   {\n";
  out << "    if (to_underlying(record_id) < 0 || (max_record_id && to_underlying(record_id) >= max_record_id))\n";
  out << "     throw_exception(\"insert_into: too big\");\n";
  {
   bool first = true;
   for (const auto &[tid, tname]: tables)
   {
    out << "    ";
    if (first)
     first = false;
    else
     out << "else ";

    out << "if (table_id == Table_Id(" << tid << "))\n";
    out << "    {\n";
    out << "     if (is_valid_record_id_for_" << tname << "(record_id))\n";
    out << "      throw_exception(\"Duplicate insert into table " << tname << "\");\n";
    out << "     if (storage_of_" << tname << ".size() <= size_t(record_id))\n";
    out << "      storage_of_" << tname << ".resize(to_underlying(record_id) + 1);\n";
    out << "     internal_insert_" << tname << "(record_id);\n";
    out << "    }\n";
   }
  }
  out << "   }\n";

  //
  // insert_vector
  //
  out << R"RRR(
   void delete_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override
   {
    joedb::Freedom_Keeper *fk = nullptr;

)RRR";

  {
   bool first = true;
   for (const auto &[tid, tname]: tables)
   {
    out << "    ";
    if (first)
     first = false;
    else
     out << "else ";

    out << "if (table_id == Table_Id(" << tid << "))\n";
    out << "     fk = &storage_of_" << tname << ".freedom_keeper;\n";
   }
  }

  out << R"RRR(
    if (fk)
    {
     JOEDB_RELEASE_ASSERT(fk->is_used_vector(record_id, size));
     joedb::Writable::delete_vector(table_id, record_id, size);
    }
   }

   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) override
   {
    if
    (
     to_underlying(record_id) < 0 ||
     (max_record_id && (to_underlying(record_id) >= max_record_id || joedb::index_t(size) >= max_record_id))
    )
    {
     throw_exception("insert_vector: null record_id, or too big");
    }
)RRR";

  {
   bool first = true;
   for (const auto &[tid, tname]: tables)
   {
    out << "    ";
    if (first)
     first = false;
    else
     out << "else ";

    out << "if (table_id == Table_Id(" << tid << "))\n";
    out << "    {\n";
    out << "     if (storage_of_" << tname << ".size() < size_t(record_id) + size)\n";
    out << "      storage_of_" << tname << ".resize(to_underlying(record_id) + size);\n";
    out << "     internal_vector_insert_" << tname << "(record_id, size);\n";
    out << "    }\n";
   }
  }
  out << "   }\n";

  //
  // set of existing types in the database
  //
  std::set<Type::Type_Id> db_types;

  for (const auto &[tid, tname]: tables)
   for (const auto &[fid, fname]: db.get_fields(tid))
    db_types.insert(db.get_field_type(tid, fid).get_type_id());

  //
  // update
  //
  {
   for (int type_index = 1; type_index < int(Type::type_ids); type_index++)
   {
    const Type::Type_Id type_id = Type::Type_Id(type_index);
    if (db_types.find(type_id) == db_types.end())
     continue;

    out << '\n';
    out << "   void update_" << get_type_string(type_id) << '\n';
    out << "   (\n";
    out << "    Table_Id table_id,\n";
    out << "    Record_Id record_id,\n";
    out << "    Field_Id field_id,\n";
    out << "    " << get_cpp_type_string(type_id) << " value\n";
    out << "   )\n";
    out << "   override\n";
    out << "   {\n";

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
      out << "    if (table_id == Table_Id(" << tid << "))\n";
      out << "    {\n";

      for (const auto &[fid, fname]: db.get_fields(tid))
      {
       const Type &type = db.get_field_type(tid, fid);
       if (type.get_type_id() == type_id)
       {
        out << "     if (field_id == Field_Id(" << fid << "))\n";
        out << "     {\n";
        out << "      internal_update_" << tname;
        out << "__" << fname << "(record_id, ";
        if (type.get_type_id() != Type::Type_Id::reference)
         out << "value";
        else
        {
         out << "id_of_" << db.get_table_name(type.get_table_id());
         out << "(value)";
        }
        out << ");\n";
        out << "      return;\n";
        out << "     }\n";
       }
      }

      out << "     return;\n";
      out << "    }\n";
     }
    }

    out << "   }\n";
   }
  }

  //
  // update_vector
  //
  {
   for (int type_index = 1; type_index < int(Type::type_ids); type_index++)
   {
    const auto type_id = Type::Type_Id(type_index);
    if (db_types.find(Type::Type_Id(type_id)) == db_types.end())
     continue;

    out << '\n';
    out << "   void update_vector_" << get_type_string(type_id) << '\n';
    out << "   (\n";
    out << "    Table_Id table_id,\n";
    out << "    Record_Id record_id,\n";
    out << "    Field_Id field_id,\n";
    out << "    size_t size,\n";
    out << "    const " << get_storage_type_string(type_id) << " *value\n";
    out << "   )\n";
    out << "   override\n";
    out << "   {\n";

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
      out << "    if (table_id == Table_Id(" << tid << "))\n";
      out << "    {\n";

      for (const auto &[fid, fname]: db.get_fields(tid))
      {
       const Type &type = db.get_field_type(tid, fid);
       if (type.get_type_id() == type_id)
       {
        out << "     if (field_id == Field_Id(" << fid << "))\n";
        out << "     {\n";
        out << "      internal_update_vector_" << tname;
        out << "__" << fname << "(record_id, size, ";

        if (type_id != joedb::Type::Type_Id::reference)
         out << "value";
        else
        {
         out << "reinterpret_cast<const ";
         write_type(out, type, false, false);
         out << "*>(value)";
        }

        out << ");\n";
        out << "      return;\n";
        out << "     }\n";
       }
      }

      out << "     return;\n";
      out << "    }\n";
     }
    }

    out << "   }\n";
   }
  }

  //
  // get_own_storage
  //
  {
   for (int type_index = 1; type_index < int(Type::type_ids); type_index++)
   {
    const Type::Type_Id type_id = Type::Type_Id(type_index);
    if (db_types.find(Type::Type_Id(type_id)) == db_types.end())
     continue;

    out << '\n';
    out << "   " << get_storage_type_string(type_id);
    out << " *get_own_" << get_type_string(type_id) << "_storage\n";
    out << "   (\n";
    out << "    Table_Id table_id,\n";
    out << "    Record_Id record_id,\n";
    out << "    Field_Id field_id,\n";
    out << "    size_t &capacity\n";
    out << "   )\n";
    out << "   override\n";
    out << "   {\n";

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
      out << "    if (table_id == Table_Id(" << tid << "))\n";
      out << "    {\n";
      out << "     capacity = size_t(storage_of_" << tname << ".freedom_keeper.size());\n";

      for (const auto &[fid, fname]: db.get_fields(tid))
      {
       const Type &type = db.get_field_type(tid, fid);
       if (type.get_type_id() == type_id)
       {
        out << "     if (field_id == Field_Id(" << fid << "))\n"
            << "     {\n"
            << "      return ";

        if (type_id == Type::Type_Id::reference)
         out << "reinterpret_cast<Record_Id *>";

        out << "(storage_of_" << tname;
        out << ".field_value_of_" << fname << ".data() + to_underlying(record_id));\n"
            << "     }\n";
       }
      }

      out << "     return nullptr;\n";
      out << "    }\n";
     }
    }

    out << "    return nullptr;\n";
    out << "   }\n";
   }
  }

  //
  // Informative events are ignored
  //
  out << R"RRR(
   void comment(const std::string &comment) override {}
   void timestamp(int64_t timestamp) override {}
   void valid_data() override {}
)RRR";

  //
  // Schema changes are forwarded to the schema string
  //
  out << R"RRR(
   bool upgrading_schema = false;
   joedb::Memory_File schema_file;
   joedb::Writable_Journal schema_journal;

   bool requires_schema_upgrade() const
   {
    return schema_file.get_data().size() < detail::schema_string_size;
   }

   void check_schema()
   {
    constexpr size_t pos = joedb::Header::size;
    const size_t schema_file_size = schema_file.get_data().size();

    if
    (
     schema_file_size < pos ||
     schema_file_size > detail::schema_string_size ||
     std::memcmp
     (
      detail::schema_string + pos,
      schema_file.get_data().data() + pos,
      schema_file_size - pos
     ) != 0
    )
    {
     throw_exception("Trying to open a file with incompatible schema");
    }
   }

   void create_table(const std::string &name) override
   {
    ++current_table_id;
    schema_journal.create_table(name);
    schema_journal.soft_checkpoint();
   }

   void drop_table(Table_Id table_id) override
   {
    schema_journal.drop_table(table_id);
    schema_journal.soft_checkpoint();
   }

   void rename_table
   (
    Table_Id table_id,
    const std::string &name
   ) override
   {
    schema_journal.rename_table(table_id, name);
    schema_journal.soft_checkpoint();
   }

   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    joedb::Type type
   ) override
   {
    schema_journal.add_field(table_id, name, type);
    schema_journal.soft_checkpoint();
   }

   void drop_field(Table_Id table_id, Field_Id field_id) override
   {
    schema_journal.drop_field(table_id, field_id);
    schema_journal.soft_checkpoint();
   }

   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) override
   {
    schema_journal.rename_field(table_id, field_id, name);
    schema_journal.soft_checkpoint();
   }

   void custom(const std::string &name) override
   {
    schema_journal.custom(name);
    schema_journal.soft_checkpoint();
   }
)RRR";

  //
  // Public stuff
  //
  out << R"RRR(
  public:
   Database_Writable():
    max_record_id(0),
    schema_journal(schema_file)
   {}

   void set_max_record_id(joedb::index_t record_id)
   {
    max_record_id = record_id;
   }

   int64_t get_schema_checkpoint() const
   {
    return schema_journal.get_checkpoint();
   }

   void initialize_with_readonly_journal(joedb::Readonly_Journal &journal)
   {
    max_record_id = journal.get_checkpoint();
    journal.replay_log(*this);
    max_record_id = 0;

    check_schema();

    if (requires_schema_upgrade())
     throw_exception<joedb::Out_Of_Date>("Schema is out of date. Can't upgrade a read-only database.");
   }
  };
)RRR";

  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
