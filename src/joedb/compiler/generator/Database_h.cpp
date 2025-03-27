#include "joedb/compiler/generator/Database_h.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/get_version.h"
#include "joedb/ui/type_io.h"

#include <set>

namespace joedb::compiler::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Database_h::Database_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Database.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const interpreted::Database_Schema &db = options.get_db();
  auto tables = db.get_tables();

  namespace_include_guard(out, "Database", options.get_name_space());

  out << R"RRR(
#include "joedb/Freedom_Keeper.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/error/Exception.h"
#include "joedb/error/Out_Of_Date.h"
#include "joedb/error/assert.h"
#include "joedb/get_version.h"
#include "ids.h"

#include <string>
#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string_view>

)RRR";

  if (options.has_index())
   out << "#include <map>\n\n";

  if (options.has_unique_index())
  {
   out << "#include \"joedb/ui/type_io.h\"\n";
   out << "#include <sstream>\n\n";
  }

  out << "static_assert(std::string_view(joedb::get_version()) == \"";
  out << joedb::get_version() << "\");\n\n";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 using joedb::Record_Id;
 using joedb::Table_Id;
 using joedb::Field_Id;

 extern const char * schema_string;
 inline constexpr size_t schema_string_size = )RRR";

  out << options.schema_file.get_size() << ";\n";

  for (const auto &[tid, tname]: tables)
   out << " class container_of_" << tname << ";\n";

  for (const auto &[tid, tname]: tables)
  {
   out << "\n struct data_of_" << tname;

   out <<"\n {\n";
   if (db.get_freedom(tid).size() > 0)
    out <<"  Field_Id current_field_id = Field_Id(0);\n";

   std::vector<std::string> fields;

   for (const auto &[fid, fname]: db.get_fields(tid))
   {
    fields.emplace_back("field_value_of_" + fname);

    const joedb::Type &type = db.get_field_type(tid, fid);

    out << "  std::vector<";
    write_type(type, false, false);
    out << "> " << fields.back() << ";\n";
   }

   for (const auto &index: options.get_indices())
    if (index.table_id == tid)
    {
     out << "  std::vector<";
     write_index_type(index);
     out << "::iterator> ";
     fields.emplace_back("iterator_over_" + index.name);
     out << fields.back() << ";\n";
    }

  out << R"RRR(

  joedb::Compact_Freedom_Keeper freedom_keeper;

  size_t size() const {return freedom_keeper.size();}

  void resize(size_t new_size)
  {
)RRR";

   fields.emplace_back("freedom_keeper");
   for (const std::string &field: fields)
    out << "   " << field << ".resize(new_size);\n";

   out << "  }\n };\n\n";
  }

  for (const auto &index: options.get_indices())
   if (!index.unique)
    out << " class range_of_" << index.name << ";\n";
  out << '\n';

  out << " class Database: public joedb::Writable\n {\n";
  out << "  friend class Readable;\n";

  for (const auto &[tid, tname]: tables)
  {
   out << "  friend class id_of_"  << tname << ";\n";
   out << "  friend class container_of_"  << tname << ";\n";
  }

  for (const auto &index: options.get_indices())
   if (!index.unique)
    out << "  friend class range_of_" << index.name << ";\n";

  out << R"RRR(
  public:
   template<typename E = joedb::error::Exception>
   static void throw_exception(const std::string &message)
   {
    throw E(")RRR" <<
    namespace_string(options.get_name_space())
    << R"RRR(: " + message);
   }

   size_t max_record_id;
   Table_Id current_table_id = Table_Id{0};

   void set_max_record_id(size_t record_id)
   {
    max_record_id = record_id;
   }

)RRR";

  //
  // Validity checks
  //
  for (const auto &[tid, tname]: tables)
  {
   out << "   bool is_valid(id_of_" << tname << " id) const {return is_valid_record_id_for_" << tname << "(id.get_record_id());}\n";
  }

  out << "\n  protected:\n";

  for (const auto &[tid, tname]: tables)
  {
   out << "   data_of_" << tname << " storage_of_" << tname << ";\n";

   out << "   bool is_valid_record_id_for_" << tname;
   out << "(Record_Id record_id) const {return storage_of_" << tname;
   out << ".freedom_keeper.is_used(size_t(record_id) + 1);}\n";
  }

  //
  // Indices
  //
  if (!options.get_indices().empty())
   out << '\n';

  for (const auto &index: options.get_indices())
  {
   const std::string &tname = db.get_table_name(index.table_id);

   out << "   ";
   write_index_type(index);
   out << " index_of_" << index.name << ";\n";

   out << "   void remove_index_of_" << index.name << "(Record_Id record_id)\n";
   out << "   {\n";
   out << "    auto &iterator = storage_of_" << tname;
   out << ".iterator_over_" << index.name << "[size_t(record_id) - 1];\n";
   out << "    if (iterator != index_of_" << index.name << ".end())\n";
   out << "    {\n";
   out << "     index_of_" << index.name << ".erase(iterator);\n";
   out << "     iterator = index_of_" << index.name << ".end();\n";
   out << "    }\n";
   out << "   }\n";

   out << "   void add_index_of_" << index.name << "(Record_Id record_id)\n";
   out << "   {\n";
   out << "    auto result = index_of_" << index.name;
   out << ".insert\n    (\n     ";
   write_index_type(index);
   out << "::value_type\n     (\n      ";
   write_tuple_type(index);
   out << '(';
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";
    out << "storage_of_" << tname << ".field_value_of_";
    out << db.get_field_name(index.table_id, index.field_ids[i]);
    out << "[size_t(record_id) - 1]";
   }
   out << ')';
   out << ",\n      id_of_" << tname << "(record_id)\n     )\n    );\n";
   if (index.unique)
   {
    out << "    if (!result.second)\n";
    out << "    {\n";
    out << "     std::ostringstream out;\n";
    out << "     out << \"" << index.name << " unique index failure: (\";\n";
    for (size_t i = 0; i < index.field_ids.size(); i++)
    {
     if (i > 0)
      out << "     out << \", \";\n";
     const auto type = db.get_field_type(index.table_id, index.field_ids[i]);
     out << "     joedb::ui::write_" << get_type_string(type) << "(out, ";
     out << "storage_of_" << tname << ".field_value_of_";
     out << db.get_field_name(index.table_id, index.field_ids[i]);
     out << "[size_t(record_id) - 1]";
     if (type.get_type_id() == joedb::Type::Type_Id::reference)
      out << ".get_record_id()";
     out << ");\n";
    }
    out << "     out << \") at id = \" << record_id << ' ';\n";
    out << "     out << \"was already at id = \" << result.first->second.get_id();\n";
    out << "     throw_exception(out.str());\n";
    out << "    }\n";
    out << "    storage_of_" << tname << ".iterator_over_" << index.name << "[size_t(record_id) - 1] = result.first;\n";
   }
   else
    out << "    storage_of_" << tname << ".iterator_over_" << index.name << "[size_t(record_id) - 1] = result;\n";
   out << "   }\n";
  }

  //
  // Internal data-modification functions
  //
  out << '\n';
  for (const auto &[tid, tname]: tables)
  {
   out << "   void internal_delete_" << tname << "(Record_Id record_id)\n";
   out << "   {\n";
   out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record_id));\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == tid)
     out << "    remove_index_of_" << index.name << "(record_id);\n";

   for (const auto &[fid, fname]: db.get_fields(tid))
   {
    const Type &type = db.get_field_type(tid, fid);

    out << "    storage_of_" << tname << ".field_value_of_";
    out << fname << "[size_t(record_id) - 1]";

    if (type.get_type_id() == Type::Type_Id::string)
    {
     out << ".clear()";
    }
    else if (type.get_type_id() == Type::Type_Id::reference)
    {
     out << " = ";
     write_type(type, false, false);
     out << "(Record_Id(0))";
    }
    else if (type.get_type_id() == Type::Type_Id::blob)
    {
     out << " = joedb::Blob();";
    }
    else
    {
     out << " = 0";
    }

    out << ";\n";
   }

   out << "    storage_of_" << tname << ".freedom_keeper.free(size_t(record_id) + 1);\n";
   out << "   }\n";
  }

  out << '\n';
  for (const auto &[tid, tname]: tables)
  {
   out << "   void internal_insert_" << tname << "(Record_Id record_id)\n";
   out << "   {\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == tid)
    {
     out << "    storage_of_" << tname;
     out << ".iterator_over_" << index.name << "[size_t(record_id) - 1] = ";
     out << "index_of_" << index.name << ".end();\n";
    }

   out << "    storage_of_" << tname << ".freedom_keeper.use(size_t(record_id) + 1);\n";

   out << "   }\n\n";

   out << "   void internal_vector_insert_" << tname << "(Record_Id record_id, size_t size)\n";
   out << "   {\n";
   out << "    storage_of_" << tname << ".freedom_keeper.use_vector(size_t(record_id) + 1, size);\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == tid)
    {
     out << "    std::fill_n\n";
     out << "    (\n";
     out << "     &storage_of_" << tname << ".iterator_over_" << index.name << "[size_t(record_id) - 1],\n";
     out << "     size,\n";
     out << "     index_of_" << index.name << ".end()\n";
     out << "    );\n";
    }

   out << "   }\n";
  }

  out << '\n';
  for (const auto &[tid, tname]: tables)
  {
   for (const auto &[fid, fname]: db.get_fields(tid))
   {
    const Type &type = db.get_field_type(tid, fid);

    out << "   void internal_update_" << tname << "__" << fname;
    out << "\n   (\n    Record_Id record_id,\n    ";
    write_type(type, true, false);
    out << " field_value_of_" << fname << "\n   )\n";
    out << "   {\n";
    out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record_id));\n";
    out << "    storage_of_" << tname << ".field_value_of_" << fname;
    out << "[size_t(record_id) - 1] = field_value_of_" << fname;
    out << ";\n";

    for (const auto &index: options.get_indices())
    {
     if (index.is_trigger(tid, fid))
     {
      out << "    remove_index_of_" << index.name << "(record_id);\n";
      out << "    add_index_of_" << index.name << "(record_id);\n";
     }
    }

    out << "   }\n\n";

    out << "   void internal_update_vector_" << tname << "__" << fname << '\n';
    out << "   (\n";
    out << "    Record_Id record_id,\n";
    out << "    size_t size,\n";
    out << "    const ";
    write_type(type, false, false);
    out << " *value\n";
    out << "   )\n";
    out << "   {\n";
    out << "    for (size_t i = 0; i < size; i++)\n";
    out << "     JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record_id + i));\n";
    out << "    ";
    write_type(type, false, false);
    out << " *target = &storage_of_" << tname;
    out << ".field_value_of_" << fname << "[size_t(record_id) - 1];\n";
    out << "    if (target != value)\n";
    out << "     std::copy_n(value, size, target);\n";

    for (const auto &index: options.get_indices())
    {
     if (index.is_trigger(tid, fid))
     {
      out << "    for (size_t i = 0; i < size; i++)\n";
      out << "     remove_index_of_" << index.name << "(record_id + i);\n";
      out << "    for (size_t i = 0; i < size; i++)\n";
      out << "     add_index_of_" << index.name << "(record_id + i);\n";
     }
    }

    out << "   }\n\n";
   }
  }

  //
  // delete_from writable function
  //
  out << '\n';
  out << "   void delete_from(Table_Id table_id, Record_Id record_id) final\n";
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
  out << "   void insert_into(Table_Id table_id, Record_Id record_id) final\n";
  out << "   {\n";
  out << "    if (size_t(record_id) <= 0 || (max_record_id && size_t(record_id) > max_record_id))\n";
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
    out << "     if (storage_of_" << tname << ".size() < size_t(record_id))\n";
    out << "      storage_of_" << tname << ".resize(size_t(record_id));\n";
    out << "     internal_insert_" << tname << "(record_id);\n";
    out << "    }\n";
   }
  }
  out << "   }\n";

  //
  // insert_vector
  //
  out << R"RRR(

   void insert_vector
   (
    Table_Id table_id,
    Record_Id record_id,
    size_t size
   ) final
   {
    if
    (
     size_t(record_id) <= 0 ||
     (max_record_id && (size_t(record_id) > max_record_id || size > max_record_id))
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
    out << "     if (storage_of_" << tname << ".size() < size_t(record_id) + size - 1)\n";
    out << "      storage_of_" << tname << ".resize(size_t(record_id) + size - 1);\n";
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
    out << "   final\n";
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
    out << "   final\n";
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
         write_type(type, false, false);
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
    out << "   final\n";
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
        out << ".field_value_of_" << fname << ".data() + size_t(record_id) - 1);\n"
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
   void comment(const std::string &comment) override {};
   void timestamp(int64_t timestamp) override {};
   void valid_data() final {};
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
    return schema_file.get_data().size() < schema_string_size;
   }

   void check_schema()
   {
    constexpr size_t pos = size_t(joedb::Writable_Journal::header_size);
    const size_t schema_file_size = schema_file.get_data().size();

    if
    (
     schema_file_size < pos ||
     schema_file_size > schema_string_size ||
     std::memcmp
     (
      schema_file.get_data().data() + pos,
      schema_string + pos,
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
    schema_journal.default_checkpoint();
   }

   void drop_table(Table_Id table_id) final
   {
    schema_journal.drop_table(table_id);
    schema_journal.default_checkpoint();
   }

   void rename_table
   (
    Table_Id table_id,
    const std::string &name
   ) final
   {
    schema_journal.rename_table(table_id, name);
    schema_journal.default_checkpoint();
   }

   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    joedb::Type type
   ) override
   {
    schema_journal.add_field(table_id, name, type);
    schema_journal.default_checkpoint();
   }

   void drop_field(Table_Id table_id, Field_Id field_id) final
   {
    schema_journal.drop_field(table_id, field_id);
    schema_journal.default_checkpoint();
   }

   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) final
   {
    schema_journal.rename_field(table_id, field_id, name);
    schema_journal.default_checkpoint();
   }

   void custom(const std::string &name) override
   {
    schema_journal.custom(name);
    schema_journal.default_checkpoint();
   }
)RRR";

  //
  // Public stuff
  //
  out << R"RRR(
  public:
   Database():
    max_record_id(0),
    schema_journal(schema_file)
   {}

   int64_t get_schema_checkpoint() const
   {
    return schema_journal.get_checkpoint_position();
   }

   void initialize_with_readonly_journal(joedb::Readonly_Journal &journal)
   {
    max_record_id = size_t(journal.get_checkpoint_position());
    journal.replay_log(*this);
    max_record_id = 0;

    check_schema();

    if (requires_schema_upgrade())
     throw_exception<joedb::error::Out_Of_Date>("Schema is out of date. Can't upgrade a read-only database.");
   }
)RRR";

  for (const auto &[tid, tname]: tables)
  {
   out << '\n';
   const bool single_row = options.get_table_options(tid).single_row;

   //
   // Declaration of container access
   //
   out << "   container_of_" << tname << " get_" << tname << "_table() const;\n\n";

   out << "   id_of_" << tname << " next(id_of_" << tname << " id) const\n";
   out << "   {\n";
   out << "    return id_of_" << tname << "\n    (\n     Record_Id(storage_of_" << tname << ".freedom_keeper.get_next(id.get_id() + 1) - 1)\n    );\n";
   out << "   }\n\n";

   out << "   id_of_" << tname << " previous(id_of_" << tname << " id) const\n";
   out << "   {\n";
   out << "    return id_of_" << tname << "\n    (\n     Record_Id(storage_of_" << tname << ".freedom_keeper.get_previous(id.get_id() + 1) - 1)\n    );\n";
   out << "   }\n\n";

   out << "   template<class Comparator>\n";
   out << "   std::vector<id_of_" << tname << "> sorted_" << tname;
   out << "(Comparator comparator) const;\n\n";

   //
   // Easy access to null
   //
   out << "   static id_of_" << tname << " null_" << tname << "()\n";
   out << "   {\n";
   out << "    return id_of_" << tname << "();\n";
   out << "   }\n";

   //
   // the_<table>
   //
   if (single_row)
   {
    out << "   static constexpr id_of_" << tname << " the_" << tname << "()\n";
    out << "   {\n";
    out << "    return id_of_" << tname << "{1};\n";
    out << "   }\n";
   }

   //
   // Loop over fields
   //
   for (const auto &[fid, fname]: db.get_fields(tid))
   {
    const Type &type = db.get_field_type(tid, fid);

    out << '\n';

    //
    // Getter
    //
    out << "   ";
    write_type(type, true, false);
    out << " get_" << fname << "(id_of_" << tname << " record";
    if (single_row)
     out << "= id_of_" << tname << "{1}";
    out << ") const\n";
    out << "   {\n";
    out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record.get_record_id()));\n";
    out << "    return (";
    write_type(type, true, false);
    out << ")(storage_of_" << tname;
    out << ".field_value_of_" << fname << "[record.get_id() - 1]);\n";
    out << "   }\n";
   }
  }

  //
  // get_index_of_X
  //
  for (const auto &index: options.get_indices())
  {
   out << '\n';
   out << "   const ";
   write_index_type(index);
   out << " &get_index_of_" << index.name << "()\n";
   out << "   {\n";
   out << "    return index_of_" << index.name << ";\n";
   out << "   }\n";
  }

  //
  // find_index
  //
  for (const auto &index: options.get_indices())
   if (index.unique)
   {
    const std::string &tname = db.get_table_name(index.table_id);
    out << '\n';

    out << "   id_of_" << tname << " next_" << index.name << '(';
    out << "id_of_" << tname << " id)\n";
    out << "   {\n";
    out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(id.get_record_id()));\n";
    out << "    auto iterator = storage_of_" << tname << ".iterator_over_" << index.name << "[id.get_id() - 1];\n";
    out << "    ++iterator;\n";
    out << "    if (iterator != index_of_" << index.name << ".end())\n";
    out << "     return iterator->second;\n";
    out << "    else\n";
    out << "     return id_of_" << tname << "();\n";
    out << "   }\n";

    out << "   id_of_" << tname << " previous_" << index.name << '(';
    out << "id_of_" << tname << " id)\n";
    out << "   {\n";
    out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(id.get_record_id()));\n";
    out << "    auto iterator = storage_of_" << tname << ".iterator_over_" << index.name << "[id.get_id() - 1];\n";
    out << "    if (iterator != index_of_" << index.name << ".begin())\n";
    out << "     return (--iterator)->second;\n";
    out << "    else\n";
    out << "     return id_of_" << tname << "();\n";
    out << "   }\n";

    out << "   id_of_" << tname << " find_" << index.name << '(';
    for (size_t i = 0; i < index.field_ids.size(); i++)
    {
     if (i > 0)
      out << ", ";
     const Type &type = db.get_field_type(index.table_id, index.field_ids[i]);
     write_type(type, true, false);
     out << " field_value_of_";
     out << db.get_field_name(index.table_id, index.field_ids[i]);
    }
    out << ") const\n";
    out << "   {\n";
    out << "    const auto i = index_of_" << index.name << ".find(";
    write_tuple_type(index);
    out << '(';
    for (size_t i = 0; i < index.field_ids.size(); i++)
    {
     if (i > 0)
      out << ", ";
     out << "field_value_of_";
     out << db.get_field_name(index.table_id, index.field_ids[i]);
    }
    out << "));\n";
    out << "    if (i == index_of_" << index.name << ".end())\n";
    out << "     return id_of_" << tname << "();\n";
    out << "    else\n";
    out << "     return i->second;\n";
    out << "   }\n";
   }
   else
   {
    out << "   range_of_" << index.name << " find_" << index.name << '(';
    for (size_t i = 0; i < index.field_ids.size(); i++)
    {
     if (i > 0)
      out << ", ";
     const Type &type = db.get_field_type(index.table_id, index.field_ids[i]);
     write_type(type, true, false);
     out << " field_value_of_";
     out << db.get_field_name(index.table_id, index.field_ids[i]);
    }
    out << ") const;\n";
   }

  out << " };\n";

  //
  // Plain iteration over tables
  //
  for (const auto &[tid, tname]: tables)
  {
   out << " class container_of_" << tname << "\n";
   out << " {\n";
   out << "  friend class Database;\n";
   out << '\n';
   out << "  private:\n";
   out << "   const Database &db;\n";
   out << "   container_of_" << tname << "(const Database &db): db(db) {}\n";
   out << '\n';
   out << "  public:\n";
   out << "   class iterator\n";
   out << "   {\n";
   out << "    friend class container_of_" << tname << ";\n";
   out << "    private:\n";


   out << "     const joedb::Compact_Freedom_Keeper *fk;\n"; // must use pointer for copy constructor
   out << "     size_t index;\n";
   out << "     iterator(const data_of_" << tname << " &data): fk(&data.freedom_keeper), index(0) {}\n";
   out << "    public:\n";
   out << "     typedef std::forward_iterator_tag iterator_category;\n";
   out << "     typedef id_of_" << tname << " value_type;\n";
   out << "     typedef std::ptrdiff_t difference_type;\n";
   out << "     typedef value_type* pointer;\n";
   out << "     typedef value_type& reference;\n";
   out << '\n';
   out << "     bool operator==(const iterator &i) const {return index == i.index;}\n";
   out << "     bool operator!=(const iterator &i) const {return index != i.index;}\n";
   out << "     iterator &operator++() {index = fk->get_next(index); return *this;}\n";
   out << "     iterator operator++(int) {auto copy = *this; index = fk->get_next(index); return copy;}\n";
   out << "     iterator &operator--() {index = fk->get_previous(index); return *this;}\n";
   out << "     iterator operator--(int) {auto copy = *this; index = fk->get_previous(index); return copy;}\n";
   out << "     id_of_" << tname << " operator*() const {return id_of_";
   out << tname << "(Record_Id(index - 1));}\n";
   out << "   };\n";
   out << '\n';
   out << "   iterator begin() const {return ++iterator(db.storage_of_" << tname << ");}\n";
   out << "   iterator end() const {return iterator(db.storage_of_" << tname << ");}\n";
   out << "   bool is_empty() const {return db.storage_of_" << tname
       << ".freedom_keeper.is_empty();}\n";
   out << "   size_t get_size() const {return db.storage_of_" << tname << ".freedom_keeper.get_used_count();}\n";
   out << "   static id_of_" << tname << " get_at(size_t i) {return id_of_"
       << tname << "(Record_Id(i));}\n";
   out << "   bool is_valid_at(size_t i) {return db.storage_of_" << tname << ".freedom_keeper.is_used(i + 1);}\n";

   out << "   id_of_" << tname << " first() const {return *begin();}\n";
   out << "   id_of_" << tname << " last() const {return *--end();}\n";
   out << "   id_of_" << tname << " get_end() const {return *end();}\n";

   out << " };\n";
   out << '\n';

   out << " inline container_of_" << tname << " Database::get_" << tname << "_table() const\n";
   out << " {\n";
   out << "  return container_of_" << tname << "(*this);\n";
   out << " }\n";
   out << '\n';

   out << " template<class Comparator>\n";
   out << " std::vector<id_of_" << tname << "> Database::sorted_" << tname;
   out << "(Comparator comparator) const\n";
   out << " {\n";
   out << "  std::vector<id_of_" << tname << "> result;\n";
   out << "  for (auto x: get_" << tname << "_table())\n";
   out << "   result.emplace_back(x);\n";
   out << "  std::sort(result.begin(), result.end(), comparator);\n";
   out << "  return result;\n";
   out << " }\n";
  }

  //
  // Index ranges for indexes that are not unique
  //
  for (const auto &index: options.get_indices())
  {
   if (!index.unique)
   {
    out << " class range_of_" << index.name << "\n";
    out << " {\n";
    out << "  friend class Database;\n";
    out << "  private:\n";
    out << "   std::pair<";
    write_index_type(index);
    out << "::const_iterator, ";
    write_index_type(index);
    out << "::const_iterator> range;\n";
    out << "   range_of_" << index.name << "(const Database &db";
    for (size_t i = 0; i < index.field_ids.size(); i++)
    {
     out << ", ";
     const Type &type = db.get_field_type(index.table_id, index.field_ids[i]);
     write_type(type, true, false);
     out << ' ' << db.get_field_name(index.table_id, index.field_ids[i]);
    }
    out << ")\n";
    out << "   {\n";
    out << "    range = db.index_of_" << index.name << ".equal_range(";
    write_tuple_type(index);
    out << '(';
    for (size_t i = 0; i < index.field_ids.size(); i++)
    {
     if (i > 0)
      out << ", ";
     out << db.get_field_name(index.table_id, index.field_ids[i]);
    }
    out << "));\n";
    out << "   }\n";
    out << "  public:\n";
    out << "   class iterator\n";
    out << "   {\n";
    out << "    friend class range_of_" << index.name << ";\n";
    out << "    private:\n";
    out << "     ";
    write_index_type(index);
    out << "::const_iterator map_iterator;\n";
    out << "     iterator(";
    write_index_type(index);
    out << "::const_iterator map_iterator): map_iterator(map_iterator) {}\n"
        << "    public:\n"
        << "     bool operator !=(const iterator &i) const\n"
        << "     {\n"
        << "      return map_iterator != i.map_iterator;\n"
        << "     }\n"
        << "     iterator &operator++() {map_iterator++; return *this;}\n"
        << "     id_of_" << db.get_table_name(index.table_id)
        << " operator*() const {return map_iterator->second;}\n"
        << "   };\n"
        << "   iterator begin() const {return range.first;}\n"
        << "   iterator end() const {return range.second;}\n"
        << "   bool empty() const {return range.first == range.second;}\n"
        << "   size_t size() const {return size_t(std::distance(range.first, range.second));}\n"
        << " };\n\n";

    out << " inline range_of_" << index.name << " Database::find_" << index.name << '(';
    for (size_t i = 0; i < index.field_ids.size(); i++)
    {
     if (i > 0)
      out << ", ";
     const Type &type = db.get_field_type(index.table_id, index.field_ids[i]);
     write_type(type, true, false);
     out << " field_value_of_";
     out << db.get_field_name(index.table_id, index.field_ids[i]);
    }
    out << ") const\n";
    out << " {\n";
    out << "  return range_of_" << index.name << "(*this";
    for (size_t i = 0; i < index.field_ids.size(); i++)
    {
     out << ", ";
     out << "field_value_of_";
     out << db.get_field_name(index.table_id, index.field_ids[i]);
    }
    out << ");\n";
    out << " }\n";
   }
  }

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
