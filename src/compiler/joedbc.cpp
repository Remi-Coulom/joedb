#include "Database.h"
#include "File.h"
#include "Stream_File.h"
#include "Journal_File.h"
#include "Selective_Listener.h"
#include "Interpreter.h"
#include "Compiler_Options.h"
#include "Compiler_Options_io.h"
#include "type_io.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
void write_type
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Database &db,
 Type type,
 bool return_type
)
{
 switch (type.get_type_id())
 {
  case Type::type_id_t::null:
   out << "void ";
  break;

  case Type::type_id_t::string:
   if (return_type)
    out << "const std::string &";
   else
    out << "std::string ";
  break;

  case Type::type_id_t::int32:
   out << "int32_t ";
  break;

  case Type::type_id_t::int64:
   out << "int64_t ";
  break;

  case Type::type_id_t::reference:
  {
   const table_id_t referred = type.get_table_id();
   out << db.get_tables().find(referred)->second.get_name() << "_t ";
  }
  break;

  case Type::type_id_t::boolean:
   out << "bool ";
  break;

  case Type::type_id_t::float32:
   out << "float ";
  break;

  case Type::type_id_t::float64:
   out << "double ";
  break;

  case Type::type_id_t::int8:
   out << "int8_t ";
  break;
 }
}

/////////////////////////////////////////////////////////////////////////////
void write_tuple_type
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Database &db,
 const Compiler_Options::Index &index
)
{
 const Table &table = db.get_tables().find(index.table_id)->second;

 out << "std::tuple<";
 for (size_t i = 0; i < index.field_ids.size(); i++)
 {
  if (i > 0)
   out << ", ";
  const Field &field = table.get_fields().find(index.field_ids[i])->second;
  write_type(out, db, field.get_type(), false);
 }
 out << ">";
}

/////////////////////////////////////////////////////////////////////////////
void write_index_type
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Database &db,
 const Compiler_Options::Index &index
)
{
 out << "std::";
 if (index.unique)
  out << "map";
 else
  out << "multimap";
 out << '<';

 write_tuple_type(out, db, index);

 const Table &table = db.get_tables().find(index.table_id)->second;
 out << ", " << table.get_name() << "_t>";
}

/////////////////////////////////////////////////////////////////////////////
void generate_h(std::ostream &out, const Compiler_Options &options)
/////////////////////////////////////////////////////////////////////////////
{
#define STRINGIFY(X) #X
#define EXPAND_AND_STRINGIFY(X) STRINGIFY(X)

 char const * const types[] =
 {
  0,
#define TYPE_MACRO(a, b, type_id, d, e) EXPAND_AND_STRINGIFY(type_id),
#include "TYPE_MACRO.h"
#undef TYPE_MACRO
 };

 char const * const cpp_types[] =
 {
  0,
#define TYPE_MACRO(a, type, c, d, e) EXPAND_AND_STRINGIFY(type)" ",
#include "TYPE_MACRO.h"
#undef TYPE_MACRO
 };

 char const * const storage_types[] =
 {
  0,
#define TYPE_MACRO(storage, b, c, d, e) EXPAND_AND_STRINGIFY(storage),
#include "TYPE_MACRO.h"
#undef TYPE_MACRO
 };

#undef EXPAND_AND_STRINGIFY
#undef STRINGIFY

 const Database &db = options.get_db();
 auto tables = db.get_tables();

 out << "#ifndef " << options.get_namespace_name() << "_Database_declared\n";
 out << "#define " << options.get_namespace_name() << "_Database_declared\n";
 out << R"RRR(
#include <string>
#include <cstdint>
#include <vector>
#include <cassert>
#include <map>
#include <algorithm>

#include "joedb/File.h"
#include "joedb/Journal_File.h"
#include "joedb/Database.h"
#include "joedb/Dummy_Listener.h"
#include "joedb/Freedom_Keeper.h"

)RRR";

 out << "namespace " << options.get_namespace_name() << "\n{\n";
 out << " class Database;\n\n";

 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  out << " class " << tname << "_container;\n";
 }

 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  out << '\n';
  out << " class " << tname << "_t\n {\n";
  out << "  friend class Database;\n";
  for (auto &friend_table: tables)
   if (friend_table.first != table.first)
    out << "  friend class " << friend_table.second.get_name() << "_t;\n";
  out << "  friend class "  << tname << "_container;\n";
  out << "\n  private:\n";
  out << "   record_id_t id;\n";
  out << "\n  public:\n";
  out << "   explicit " << tname << "_t(record_id_t id): id(id) {}\n";
  out << "   " << tname << "_t(): id(0) {}\n";
  out << "   bool is_null() const {return id == 0;}\n";
  out << "   record_id_t get_id() const {return id;}\n";
  out << "   bool operator==(" << tname << "_t x) const {return id == x.id;}\n";
  out << "   bool operator<(" << tname << "_t x) const {return id < x.id;}\n";
  out << "   " << tname << "_t operator[](record_id_t i) const {return " << tname << "_t(id + i);}\n";
  out << " };\n";
 }

 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  const auto storage = options.get_table_options(table.first).storage;

  out << "\n struct " << tname << "_data";

  if (storage == Compiler_Options::Table_Storage::freedom_keeper)
   out << ": public joedb::EmptyRecord";

  out <<"\n {\n";

  if (storage == Compiler_Options::Table_Storage::freedom_keeper)
  {
   out << "  " << tname << "_data() {}\n";
   out << "  " << tname << "_data(bool f): joedb::EmptyRecord(f) {}\n";
  }

  for (const auto &field: table.second.get_fields())
  {
   out << "  ";
   write_type(out, db, field.second.get_type(), false);
   out << field.second.get_name() << ";\n";
  }

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
   {
    out << "  ";
    write_index_type(out, db, index);
    out << "::iterator ";
    out << index.name << "_iterator;\n";
   }

  out << " };\n\n";
 }

 for (auto &index: options.get_indices())
  if (!index.unique)
   out << " class " << index.name << "_range;\n";
 out << '\n';

 out << " class Database: public joedb::Listener\n {\n";

 for (auto &table: tables)
 {
  out << "  friend class "  << table.second.get_name() << "_t;\n";
  out << "  friend class "  << table.second.get_name() << "_container;\n";
 }

 for (auto &index: options.get_indices())
  if (!index.unique)
   out << "  friend class " << index.name << "_range;\n";

 out << R"RRR(
  protected:
   joedb::Dummy_Listener dummy_listener;

  private:
   joedb::Listener *listener;

)RRR";

 //
 // Vectors, and freedom keepers
 //
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  const auto storage = options.get_table_options(table.first).storage;

  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "   joedb::Freedom_Keeper<" << tname << "_data>";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "   std::vector<" << tname << "_data>";
   break;

   default:
    out << "   // Error. storage = " << int(storage) << '\n';
    out << "   // table_id = " << table.first << '\n';
   break;
  }

  out << " storage_of_" << tname << ";\n";
 }

 //
 // Indices
 //
 if (options.get_indices().size())
  out << '\n';

 for (const auto &index: options.get_indices())
 {
  const Table &table = db.get_tables().find(index.table_id)->second;
  const std::string &tname = table.get_name();

  out << "   ";
  write_index_type(out, db, index);
  out << ' ' << index.name << ";\n";

  out << "   void " << index.name << "_remove_index(record_id_t record_id)\n";
  out << "   {\n";
  out << "    auto &iterator = storage_of_" << tname;
  out << ".get_record(record_id + 1)." << index.name << "_iterator;\n";
  out << "    if (iterator != " << index.name << ".end())\n";
  out << "    {\n";
  out << "     " << index.name << ".erase(iterator);\n";
  out << "     iterator = " << index.name << ".end();\n";
  out << "    }\n";
  out << "   }\n";

  out << "   void " << index.name << "_add_index(record_id_t record_id)\n";
  out << "   {\n";
  out << "    " << tname << "_data &data = storage_of_";
  out << tname << ".get_record(record_id + 1);\n";
  out << "    auto result = " << index.name;
  out << ".insert\n    (\n     ";
  write_index_type(out, db, index);
  out << "::value_type\n     (\n      ";
  write_tuple_type(out, db, index);
  out << '(';
  for (size_t i = 0; i < index.field_ids.size(); i++)
  {
   if (i > 0)
    out << ", ";
   const Field &field =
    table.get_fields().find(index.field_ids[i])->second;
   out << "data." << field.get_name();
  }
  out << ')';
  out << ",\n      " << tname << "_t(record_id)\n     )\n    );\n";
  if (index.unique)
  {
   out << "    if (!result.second)\n";
   out << "     throw std::runtime_error(\"";
   out << index.name << " unique index failure\");\n";
   out << "    data." << index.name << "_iterator = result.first;\n";
  }
  else
   out << "    data." << index.name << "_iterator = result;\n";
  out << "   }\n";
 }

 //
 // Internal data-modification functions
 //
 out << '\n';
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  const auto storage = options.get_table_options(table.first).storage;
  const bool has_delete = storage == Compiler_Options::Table_Storage::freedom_keeper;

  if (has_delete)
  {
   out << "   void internal_delete_" << tname << "(record_id_t record_id)\n";
   out << "   {\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == table.first)
     out << "    " << index.name << "_remove_index(record_id);\n";

   out << "    storage_of_" << tname << ".free(record_id + 1);\n";
   out << "   }\n";
  }
 }

 out << '\n';
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  auto storage = options.get_table_options(table.first).storage;

  out << "   void internal_insert_" << tname << "(record_id_t record_id)\n";
  out << "   {\n";

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
   {
    out << "    " << tname << "_data &data = storage_of_";
    out << tname << ".get_record(record_id + 1);\n";
    out << "    data." << index.name << "_iterator = ";
    out << index.name << ".end();\n";
   }

  if (storage == Compiler_Options::Table_Storage::freedom_keeper)
   out << "    storage_of_" << tname << ".use(record_id + 1);\n";
  out << "   }\n";
 }

 out << '\n';
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  for (auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();
   out << "   void internal_update_" << tname << '_' << fname;
   out << "\n   (\n    record_id_t record_id,\n    ";
   write_type(out, db, field.second.get_type(), true);
   out << fname << "\n   )\n";
   out << "   {\n";
   out << "    storage_of_" << tname << "[record_id - 1]." << fname;
   out << " = " << fname << ";\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == table.first &&
        std::find(index.field_ids.begin(),
                  index.field_ids.end(),
                  field.first) != index.field_ids.end())
    {
     out << "    " << index.name << "_remove_index(record_id);\n";
     out << "    " << index.name << "_add_index(record_id);\n";
    }
   out << "   }\n";
  }
 }

 //
 // after_delete listener function
 //
 out << '\n';
 out << "   void after_delete(table_id_t table_id, record_id_t record_id) override\n";
 out << "   {\n";
 {
  bool first = true;
  for (auto table: tables)
  {
   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   out << "if (table_id == " << table.first << ")\n";

   const std::string &name = table.second.get_name();
   const auto storage = options.get_table_options(table.first).storage;

   switch(storage)
   {
    case Compiler_Options::Table_Storage::freedom_keeper:
     out << "     internal_delete_" << name << "(record_id);\n";
    break;

    case Compiler_Options::Table_Storage::vector:
     out << "     throw std::runtime_error(\"No delete allowed on vector table: " << name << "\");\n";
    break;
   }
  }
 }
 out << "   }\n";

 //
 // after_insert
 //
 out << '\n';
 out << "   void after_insert(table_id_t table_id, record_id_t record_id) override\n";
 out << "   {\n";
 {
  bool first = true;
  for (auto &table: tables)
  {
   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   const std::string &name = table.second.get_name();
   out << "if (table_id == " << table.first << ")\n";
   out << "    {\n";
   out << "     if (storage_of_" << name << ".size() < record_id)\n";
   out << "      storage_of_" << name << ".resize(record_id);\n";
   out << "     internal_insert_" << name << "(record_id);\n";
   out << "    }\n";
  }
 }
 out << "   }\n";

 //
 // after_insert_vector
 //
 out << '\n';
 out << "   void after_insert_vector(table_id_t table_id, record_id_t record_id, record_id_t size) override\n";
 out << "   {\n";
 {
  bool first = true;
  for (auto &table: tables)
  {
   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   const std::string &name = table.second.get_name();
   out << "if (table_id == " << table.first << ")\n";
   out << "    {\n";
   out << "     if (storage_of_" << name << ".size() < record_id + size - 1)\n";
   out << "      storage_of_" << name << ".resize(record_id + size - 1);\n";
   out << "     for (record_id_t i = 0; i < size; i++)\n";
   out << "      internal_insert_" << name << "(record_id + i);\n";
   out << "    }\n";
  }
 }
 out << "   }\n";

 //
 // after_update
 //
 {
  for (int type_id = 1; type_id < int(Type::type_ids); type_id++)
  {
   out << '\n';
   out << "   void after_update_" << types[type_id] << '\n';
   out << "   (\n";
   out << "    table_id_t table_id,\n";
   out << "    record_id_t record_id,\n";
   out << "    field_id_t field_id,\n";
   out << "    " << cpp_types[type_id] << "value\n";
   out << "   )\n";
   out << "   override\n";
   out << "   {\n";

   for (auto &table: tables)
   {
    bool has_typed_field = false;

    for (auto &field: table.second.get_fields())
     if (int(field.second.get_type().get_type_id()) == type_id)
     {
      has_typed_field = true;
      break;
     }

    if (has_typed_field)
    {
     out << "    if (table_id == " << table.first << ")\n";
     out << "    {\n";

     for (auto &field: table.second.get_fields())
      if (int(field.second.get_type().get_type_id()) == type_id)
      {
       out << "     if (field_id == " << field.first << ")\n";
       out << "     {\n";
       out << "      internal_update_" << table.second.get_name();
       out << '_' << field.second.get_name() << "(record_id, ";
       if (field.second.get_type().get_type_id() != Type::type_id_t::reference)
        out << "value";
       else
       {
        const table_id_t table_id = field.second.get_type().get_table_id();
        out << tables.find(table_id)->second.get_name();
        out << "_t(value)";
       }
       out << ");\n";
       out << "      return;\n";
       out << "     }\n";
      }

     out << "     return;\n";
     out << "    }\n";
    }
   }

   out << "   }\n";
  }
 }

 //
 // after_update_vector
 //
 {
  for (int type_id = 1; type_id < int(Type::type_ids); type_id++)
  {
   out << '\n';
   out << "   void after_update_vector_" << types[type_id] << '\n';
   out << "   (\n";
   out << "    table_id_t table_id,\n";
   out << "    record_id_t record_id,\n";
   out << "    field_id_t field_id,\n";
   out << "    record_id_t size,\n";
   out << "    const " << storage_types[type_id] << " *value\n";
   out << "   )\n";
   out << "   override\n";
   out << "   {\n";

   for (auto &table: tables)
   {
    bool has_typed_field = false;

    for (auto &field: table.second.get_fields())
     if (int(field.second.get_type().get_type_id()) == type_id)
     {
      has_typed_field = true;
      break;
     }

    if (has_typed_field)
    {
     out << "    if (table_id == " << table.first << ")\n";
     out << "    {\n";

     for (auto &field: table.second.get_fields())
      if (int(field.second.get_type().get_type_id()) == type_id)
      {
       out << "     if (field_id == " << field.first << ")\n";
       out << "     {\n";
       out << "      for (record_id_t i = 0; i < size; i++)\n";
       out << "       internal_update_" << table.second.get_name();
       out << '_' << field.second.get_name() << "(record_id + i, ";
       if (field.second.get_type().get_type_id() != Type::type_id_t::reference)
        out << "value[i]";
       else
       {
        const table_id_t table_id = field.second.get_type().get_table_id();
        out << tables.find(table_id)->second.get_name();
        out << "_t(value[i])";
       }
       out << ");\n";
       out << "      return;\n";
       out << "     }\n";
      }

     out << "     return;\n";
     out << "    }\n";
    }
   }

   out << "   }\n";
  }
 }

 //
 // Schema changes are forwarded to the listener
 //
 out << R"RRR(

  protected:
   void after_create_table(const std::string &name) override
   {
    listener->after_create_table(name);
   }

   void after_drop_table(table_id_t table_id) override
   {
    listener->after_drop_table(table_id);
   }

   void after_rename_table(table_id_t table_id,
                           const std::string &name) override
   {
    listener->after_rename_table(table_id, name);
   }

   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        joedb::Type type) override
   {
    listener->after_add_field(table_id, name, type);
   }

   void after_drop_field(table_id_t table_id, field_id_t field_id) override
   {
    listener->after_drop_field(table_id, field_id);
   }

   void after_rename_field(table_id_t table_id,
                           field_id_t field_id,
                           const std::string &name) override
   {
    listener->after_rename_field(table_id, field_id, name);
   }

   void after_custom(const std::string &name) override
   {
    listener->after_custom(name);
   }

   void after_comment(const std::string &comment) override
   {
   }

   void after_time_stamp(int64_t time_stamp) override
   {
   }

   void after_checkpoint() override
   {
   }
)RRR";

 //
 // Public stuff
 //
 out << R"RRR(
  public:
   Database(): listener(&dummy_listener) {}

   void set_listener(Listener &new_listener) {listener = &new_listener;}
   void clear_listener() {listener = &dummy_listener;}
)RRR";

 for (auto &table: tables)
 {
  out << '\n';
  const std::string &tname = table.second.get_name();
  const auto storage = options.get_table_options(table.first).storage;
  const bool has_delete = storage == Compiler_Options::Table_Storage::freedom_keeper;

  //
  // Declaration of container access
  //
  out << "   " << tname << "_container get_" << tname << "_table() const;\n\n";
  out << "   template<class Comparator>\n";
  out << "   std::vector<" << tname << "_t> sorted_" << tname;
  out << "(Comparator comparator) const;\n\n";

  //
  // Erase all elements of the table
  //
  if (has_delete)
  {
   out << "   void clear_" << tname << "_table();\n";
   out << '\n';
  }

  //
  // Uninitialized new
  //
  out << "   " << tname << "_t new_" << tname << "()\n";
  out << "   {\n";

  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "    size_t free_record = storage_of_" << tname << ".get_free_record();\n";
    out << "    " << tname << "_t result(free_record - 1);\n\n";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "    const size_t size = storage_of_" << tname << ".size();\n";
    out << "    storage_of_" << tname << ".resize(size + 1);\n";
    out << "    " << tname << "_t result(size + 1);\n\n";
   break;
  }

  out << "    internal_insert_" << tname << "(result.id);\n\n";
  out << "    listener->after_insert(" << table.first << ", result.id);\n";
  out << "    return result;\n";
  out << "   }\n";
  out << '\n';

  //
  // new uninitialized vector
  //
  out << "   " << tname << "_t new_vector_of_" << tname << "(size_t size)\n";
  out << "   {\n";
  out << "    " << tname << "_t result(storage_of_" << tname;
  out << ".size() + 1);\n";
  out << "    storage_of_" << tname << ".resize(storage_of_";
  out << tname << ".size() + size);\n";
  out << "    for (size_t i = 0; i < size; i++)\n";
  out << "     internal_insert_" << tname << "(result.id + i);\n";
  out << "    listener->after_insert_vector(" << table.first;
  out << ", result.id, size);\n";
  out << "    return result;\n";
  out << "   }\n";
  out << '\n';

  //
  // new with all fields
  //
  out << "   " << tname << "_t new_" << tname << '\n';
  out << "   (\n    ";
  {
   bool first = true;

   for (const auto &field: table.second.get_fields())
   {
    const std::string &fname = field.second.get_name();

    if (first)
     first = false;
    else
     out << ",\n    ";

    write_type(out, db, field.second.get_type(), true);
    out << "field_" << fname;
   }

   out << '\n';
  }
  out << "   )\n";
  out << "   {\n";
  out << "    auto result = new_" << tname << "();\n";

  for (const auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();
   out << "    set_" << fname << "(result, field_" << fname << ");\n";
  }

  out << "    return result;\n";
  out << "   }\n\n";

  //
  // Delete
  //
  if (has_delete)
  {
   out << "   void delete_" << tname << "(" << tname << "_t record)\n";
   out << "   {\n";
   out << "    internal_delete_" << tname << "(record.id);\n";
   out << "    listener->after_delete(" << table.first << ", record.id);\n";
   out << "   }\n";
  }

  //
  // Loop over fields
  //
  for (const auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();

   out << '\n';

   //
   // Getter
   //
   out << "   ";
   write_type(out, db, field.second.get_type(), true);
   out << "get_" << fname << "(" << tname << "_t record) const\n";
   out << "   {\n";
   out << "    assert(!record.is_null());\n";
   out << "    return storage_of_" << tname;
   out << "[record.id - 1]." << fname << ";\n";
   out << "   }\n";

   //
   // Setter
   //
   out << "   void set_" << fname;
   out << "(" << tname << "_t record, ";
   write_type(out, db, field.second.get_type(), true);
   out << "field_" << fname << ")\n";
   out << "   {\n";
   out << "    assert(!record.is_null());\n";
   out << "    internal_update_" << tname << '_' << fname << "(record.id, ";
   out << "field_" << fname << ");\n";
   out << "    listener->after_update_";
   out << types[int(field.second.get_type().get_type_id())];
   out << '(' << table.first << ", record.id, " << field.first << ", ";
   out << "field_" << fname;
   if (field.second.get_type().get_type_id() == Type::type_id_t::reference)
    out << ".id";
   out << ");\n";
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
  write_index_type(out, db, index);
  out << " &get_index_of_" << index.name << "()\n";
  out << "   {\n";
  out << "    return " << index.name << ";\n";
  out << "   }\n";
 }

 //
 // find_index
 //
 for (const auto &index: options.get_indices())
  if (index.unique)
  {
   const Table &table = db.get_tables().find(index.table_id)->second;
   const std::string &tname = table.get_name();
   out << '\n';
   out << "   " << tname << "_t find_" << index.name << '(';
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";
    const Field &field = table.get_fields().find(index.field_ids[i])->second;
    write_type(out, db, field.get_type(), true);
    out << field.get_name();
   }
   out << ") const\n";
   out << "   {\n";
   out << "    auto i = " << index.name << ".find(";
   write_tuple_type(out, db, index);
   out << '(';
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";
    const Field &field = table.get_fields().find(index.field_ids[i])->second;
    out << field.get_name();
   }
   out << "));\n";
   out << "    if (i == " << index.name << ".end())\n";
   out << "     return " << tname << "_t();\n";
   out << "    else\n";
   out << "     return i->second;\n";
   out << "   }\n";
  }
  else
  {
   const Table &table = db.get_tables().find(index.table_id)->second;
   out << "   " << index.name << "_range find_" << index.name << '(';
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";
    const Field &field = table.get_fields().find(index.field_ids[i])->second;
    write_type(out, db, field.get_type(), true);
    out << field.get_name();
   }
   out << ") const;\n";
  }

 out << " };\n";

 //
 // File_Database
 //
 out << R"RRR(
 class File_Database: public Database
 {
  private:
   joedb::File file;
   joedb::Journal_File journal;
   static const std::string schema_string;
   bool schema_error = false;
   bool upgrading_schema = false;

   void after_custom(const std::string &name) override
   {
    Database::after_custom(name);
)RRR";
 if (db.get_custom_names().size())
 {
  out << "    if (upgrading_schema)\n";
  out << "    {\n";
  for (const auto &name: db.get_custom_names())
  {
   out << "     if (name == \"" << name << "\")\n";
   out << "      " << name << "(*this);\n";
  }
  out << "    }\n";
 }
 out << "   }\n";

 if (db.get_custom_names().size())
 {
  out << '\n';
  for (const auto &name: db.get_custom_names())
   out << "   static void " << name << "(Database &db);\n";
 }

 out << R"RRR(
  public:
   File_Database(const char *file_name, bool read_only = false);

   joedb::Journal_File::state_t get_journal_state() const
   {
    return journal.get_state();
   }

   void checkpoint_no_commit() {journal.checkpoint(0);}
   void checkpoint_half_commit() {journal.checkpoint(1);}
   void checkpoint_full_commit() {journal.checkpoint(2);}

   bool is_good() const
   {
    return file.get_status() == joedb::File::status_t::success &&
           journal.get_state() == joedb::Journal_File::state_t::no_error &&
           !schema_error;
   }
 };

)RRR";

 //
 // Plain iteration over tables
 //
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  const auto storage = options.get_table_options(table.first).storage;
  const bool has_delete = storage == Compiler_Options::Table_Storage::freedom_keeper;

  out << " class " << tname << "_container\n";
  out << " {\n";
  out << "  friend class Database;\n";
  out << '\n';
  out << "  private:\n";
  out << "   const Database &db;\n";
  out << "   " << tname << "_container(const Database &db): db(db) {}\n";
  out << '\n';
  out << "  public:\n";
  out << "   class iterator: public std::iterator<std::forward_iterator_tag, ";
  out << tname << "_t>\n";
  out << "   {\n";
  out << "    friend class " << tname << "_container;\n";
  out << "    private:\n";


  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "     const joedb::Freedom_Keeper<" << tname << "_data> &fk;\n";
    out << "     size_t index;\n";
    out << "     iterator(const joedb::Freedom_Keeper<" << tname << "_data> &fk): fk(fk), index(0) {}\n";
    out << "    public:\n";
    out << "     bool operator!=(const iterator &i) const {return index != i.index;}\n";
    out << "     iterator &operator++() {index = fk.get_next(index); return *this;}\n";
    out << "     " << tname << "_t operator*() {return ";
    out << tname << "_t(index - 1);}\n";
    out << "   };\n";
    out << '\n';
    out << "   iterator begin() {return ++iterator(db.storage_of_" << tname << ");}\n";
    out << "   iterator end() {return iterator(db.storage_of_" << tname << ");}\n";
    out << "   bool is_empty() const {return db.storage_of_" << tname
        << ".is_empty();}\n";
    out << "   size_t get_size() const {return db.storage_of_" << tname << ".get_used_count();}\n";
    out << "   static " << tname << "_t get_at(size_t i) {return "
        << tname << "_t(i);}\n";
    out << "   bool is_valid_at(size_t i) {return db.storage_of_" << tname << ".is_used(i + 1);}\n";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "     size_t index;\n";
    out << "     iterator(size_t index): index(index) {}\n";
    out << "    public:\n";
    out << "     bool operator!=(const iterator &i) const {return index != i.index;}\n";
    out << "     iterator &operator++() {index++; return *this;}\n";
    out << "     " << tname << "_t operator*() {return " << tname << "_t(index + 1);}\n";
    out << "   };\n";
    out << '\n';
    out << "   iterator begin() {return iterator(0);}\n";
    out << "   iterator end() {return iterator(db.storage_of_" << tname << ".size());}\n";
    out << "   bool is_empty() const {return db.storage_of_" << tname << ".size() == 0;}\n";
    out << "   size_t get_size() const {return db.storage_of_" << tname << ".size();}\n";
    out << "   static " << tname << "_t get_at(size_t i) {return " << tname << "_t(i);}\n";
    out << "   bool is_valid_at(size_t i) {return i > 0 && i <= db.storage_of_" << tname << ".size();}\n";
   break;
  }


  out << " };\n";
  out << '\n';

  out << " inline " << tname << "_container Database::get_" << tname << "_table() const\n";
  out << " {\n";
  out << "  return " << tname << "_container(*this);\n";
  out << " }\n";
  out << '\n';

  out << " template<class Comparator>\n";
  out << " std::vector<" << tname << "_t> Database::sorted_" << tname;
  out << "(Comparator comparator) const\n";
  out << " {\n";
  out << "  std::vector<" << tname << "_t> result;\n";
  out << "  for (auto x: get_" << tname << "_table())\n";
  out << "   result.push_back(x);\n";
  out << "  std::sort(result.begin(), result.end(), comparator);\n";
  out << "  return result;\n";
  out << " }\n";

  if (has_delete)
  {
   out << " inline void Database::clear_" << tname << "_table()\n";
   out << " {\n";
   out << "  while (!get_" << tname << "_table().is_empty())\n";
   out << "   delete_" << tname << "(*get_" << tname << "_table().begin());\n";
   out << " }\n";
   out << '\n';
  }
 }

 //
 // Index ranges for indexes that are not unique
 //
 for (const auto &index: options.get_indices())
  if (!index.unique)
  {
   const Table &table = db.get_tables().find(index.table_id)->second;

   out << " class " << index.name << "_range\n";
   out << " {\n";
   out << "  friend class Database;\n";
   out << "  private:\n";
   out << "   std::pair<";
   write_index_type(out, db, index);
   out << "::const_iterator, ";
   write_index_type(out, db, index);
   out << "::const_iterator> range;\n";
   out << "   " << index.name << "_range(const Database &db";
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    out << ", ";
    const Field &field =
     table.get_fields().find(index.field_ids[i])->second;
    write_type(out, db, field.get_type(), true);
    out << field.get_name();
   }
   out << ")\n";
   out << "   {\n";
   out << "    range = db." << index.name << ".equal_range(";
   write_tuple_type(out, db, index);
   out << '(';
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";
    const Field &field = table.get_fields().find(index.field_ids[i])->second;
    out << field.get_name();
   }
   out << "));\n";
   out << "   }\n";
   out << "  public:\n";
   out << "   class iterator: public std::iterator<std::forward_iterator_tag, ";
   out << table.get_name() << "_t>\n";
   out << "   {\n";
   out << "    friend class " << index.name << "_range;\n";
   out << "    private:\n";
   out << "     ";
   write_index_type(out, db, index);
   out << "::const_iterator map_iterator;\n";
   out << "     iterator(";
   write_index_type(out, db, index);
   out << "::const_iterator map_iterator): map_iterator(map_iterator) {}\n";
   out << "    public:\n";
   out << "     bool operator !=(const iterator &i) const\n";
   out << "     {\n";
   out << "      return map_iterator != i.map_iterator;\n";
   out << "     }\n";
   out << "     iterator &operator++() {map_iterator++; return *this;}\n";
   out << "     " << table.get_name();
   out << "_t operator*() const {return map_iterator->second;}\n";
   out << "   };\n";
   out << "   iterator begin() {return range.first;}\n";
   out << "   iterator end() {return range.second;}\n";
   out << " };\n";

   out << "   inline " << index.name << "_range Database::find_" << index.name << '(';
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";
    const Field &field = table.get_fields().find(index.field_ids[i])->second;
    write_type(out, db, field.get_type(), true);
    out << field.get_name();
   }
   out << ") const\n";
   out << "   {\n";
   out << "    return " << index.name << "_range(*this";
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    out << ", ";
    const Field &field = table.get_fields().find(index.field_ids[i])->second;
    out << field.get_name();
   }
   out << ");\n";
   out << "   }\n";
  }


 out << "}\n\n";
 out << "#endif\n";
}

/////////////////////////////////////////////////////////////////////////////
void generate_cpp
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options,
 const std::string &schema
)
{
 out << "#include \"" << options.get_namespace_name() << ".h\"\n";
 out << "#include \"joedb/Stream_File.h\"\n";
 out << '\n';
 out << "#include <sstream>\n";
 out << '\n';
 out << "using namespace " << options.get_namespace_name() << ";\n";
 out << '\n';
 out << "const std::string File_Database::schema_string(";
 write_string(out, schema);
 out << ", ";
 out << schema.size();
 out << ");\n";

 out << R"RRR(
/////////////////////////////////////////////////////////////////////////////
File_Database::File_Database(const char *file_name, bool read_only):
/////////////////////////////////////////////////////////////////////////////
 file(file_name,
      read_only ? joedb::File::mode_t::read_existing :
                  joedb::File::mode_t::write_existing),
 journal(file)
{
 //
 // If the file does not exist, create it
 //
 if (!is_good() &&
     !read_only &&
     file.get_status() == joedb::File::status_t::failure)
 {
  file.open(file_name, joedb::File::mode_t::create_new);
  if (file.get_status() == joedb::File::status_t::success)
  {
   journal.~Journal_File();
   new(&journal) joedb::Journal_File(file);
  }
 }

 //
 // If the file was opened successfully, replay the journal, and check schema
 //
 if (is_good())
 {
  std::stringstream file_schema;
  {
   joedb::Stream_File stream_file
                      (
                       file_schema,
                       joedb::Generic_File::mode_t::create_new
                      );
   joedb::Journal_File file_schema_journal(stream_file);
   set_listener(file_schema_journal);
   journal.replay_log(*this);
   clear_listener();
  }

  //
  // If schema does not match, try to upgrade it, or fail
  //
  if (is_good() && schema_string != file_schema.str())
  {
   const size_t pos = joedb::Journal_File::header_size;
   const size_t len = file_schema.str().size() - pos;

   if (file_schema.str().compare(pos, len, schema_string, pos, len) == 0)
   {
    std::stringstream schema(schema_string);
    joedb::Stream_File schema_file
                       (
                        schema,
                        joedb::Generic_File::mode_t::read_existing
                       );
    joedb::Journal_File schema_journal(schema_file);

    schema_journal.rewind();
    schema_journal.play_until(dummy_listener, file_schema.str().size());

    set_listener(journal);
    upgrading_schema = true;
    schema_journal.play_until(*this, 0);
    upgrading_schema = false;
   }
   else
    schema_error = true;
  }
 }

 set_listener(journal);
}
)RRR";
}

/////////////////////////////////////////////////////////////////////////////
void write_initial_comment(std::ostream &out, const Compiler_Options &options)
/////////////////////////////////////////////////////////////////////////////
{
 out << "/////////////////////////////////////////////////////////////////////////////\n";
 out << "//\n";
 out << "// This code was automatically generated by the joedb compiler\n";
 out << "// https://www.remi-coulom.fr/joedb/\n";
 out << "//\n";
 out << "/////////////////////////////////////////////////////////////////////////////\n";
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Open existing file from the command line
 //
 if (argc <= 2)
 {
  std::cerr << "Usage: " << argv[0] << " <file.joedbi> <file.joedbc>\n";
  return 1;
 }

 //
 // Read file.joedbi
 //
 Database db;
 std::stringstream schema;

 {
  std::ifstream joedbi_file(argv[1]);
  if (!joedbi_file.good())
  {
   std::cerr << "Error: could not open " << argv[1] << '\n';
   return 1;
  }

  Stream_File schema_file(schema, Generic_File::mode_t::create_new);
  Journal_File journal(schema_file);
  Selective_Listener schema_listener(journal, Selective_Listener::schema);

  db.set_listener(schema_listener);
  Interpreter interpreter(db);
  interpreter.main_loop(joedbi_file, std::cerr);
 }

 //
 // Read file.joedbc
 //
 std::ifstream joedbc_file(argv[2]);
 if (!joedbc_file.good())
 {
  std::cerr << "Error: could not open " << argv[2] << '\n';
  return 1;
 }

 Compiler_Options compiler_options(db);

 if (!parse_compiler_options(joedbc_file, std::cerr, compiler_options))
 {
  std::cerr << "Error: could not parse compiler options\n";
  return 1;
 }

 //
 // Generate code
 //
 {
  std::ofstream h_file(compiler_options.get_namespace_name() + ".h");
  write_initial_comment(h_file, compiler_options);
  generate_h(h_file, compiler_options);
 }
 {
  std::ofstream cpp_file(compiler_options.get_namespace_name() + ".cpp");
  write_initial_comment(cpp_file, compiler_options);
  generate_cpp(cpp_file, compiler_options, schema.str());
 }

 return 0;
}
