#include "Database.h"
#include "File.h"
#include "Interpreter.h"
#include "Compiler_Options.h"
#include "Compiler_Options_io.h"

#include <iostream>
#include <fstream>
#include <algorithm>

namespace joedb {

static char const * const cpp_value_types[] =
{
 0,
 "std::string",
 "int32_t",
 "int64_t",
 "record_id_t",
 "bool",
 "float",
 "double"
};

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
 }
}

/////////////////////////////////////////////////////////////////////////////
void write_tuple_type
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Table &table,
 const Compiler_Options::Index &index
)
{
 out << "std::tuple<";
 for (size_t i = 0; i < index.field_ids.size(); i++)
 {
  if (i > 0)
   out << ", ";
  const Field &field = table.get_fields().find(index.field_ids[i])->second;
  out << cpp_value_types[int(field.get_type().get_type_id())];
 }
 out << ">";
}

/////////////////////////////////////////////////////////////////////////////
void write_index_type
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Table &table,
 const Compiler_Options::Index &index
)
{
 out << "std::";
 if (index.unique)
  out << "map";
 else
  out << "multimap";
 out << '<';

 write_tuple_type(out, table, index);

 out << ", " << table.get_name() << "_t>";
}

/////////////////////////////////////////////////////////////////////////////
void generate_code(std::ostream &out, const Compiler_Options &options)
/////////////////////////////////////////////////////////////////////////////
{
 char const * const types[] =
 {
  0,
  "string",
  "int32",
  "int64",
  "reference",
  "boolean",
  "float32",
  "float64"
 };

 char const * const cpp_types[] =
 {
  0,
  "const std::string &",
  "int32_t ",
  "int64_t ",
  "record_id_t ",
  "bool ",
  "float ",
  "double "
 };

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
  out << " class " << tname << "_container;\n\n";
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
  out << "   bool operator==(const " << tname << "_t " << tname << ") const {return id == " << tname << ".id;}\n";
  out << " };\n";

  out << "\n struct " << tname << "_data: public joedb::EmptyRecord\n {\n"; out << "  " << tname << "_data() {}\n";
  out << "  " << tname << "_data(bool f): joedb::EmptyRecord(f) {}\n";

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
    write_index_type(out, table.second, index);
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
  private:
   joedb::Dummy_Listener dummy_listener;
   joedb::Listener *listener;

)RRR";

 //
 // Vectors, and freedom keepers
 //
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  out << "   joedb::Freedom_Keeper<" << tname << "_data> " << tname << "_FK;\n";
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
  write_index_type(out, table, index);
  out << ' ' << index.name << ";\n";

  out << "   void " << index.name << "_remove_index(record_id_t record_id)\n";
  out << "   {\n";
  out << "    " << index.name << ".erase(" << tname;
  out << "_FK.get_record(record_id + 1)." << index.name << "_iterator);\n";
  out << "   }\n";

  out << "   void " << index.name << "_add_index(record_id_t record_id)\n";
  out << "   {\n";
  out << "    " << tname << "_data &data = ";
  out << tname << "_FK.get_record(record_id + 1);\n";
  out << "    auto result = " << index.name;
  out << ".insert\n    (\n     ";
  write_index_type(out, table, index);
  out << "::value_type\n     (\n      ";
  write_tuple_type(out, table, index);
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
   out << "    data." << index.name << "_iterator = result.first;\n";
   out << "    if (!result.second)\n";
   out << "    {\n";
   out << "     " << tname << "_t duplicate = result.first->second;\n";
   out << "     internal_delete_" << tname << "(record_id);\n";
   out << "     " << index.name << "_add_index(duplicate.id);\n";
   out << "     throw std::runtime_error(\"";
   out << index.name << " unique index failure\");\n";
   out << "    }\n";
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
  out << "   void internal_delete_" << tname << "(record_id_t record_id)\n";
  out << "   {\n";

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
    out << "    " << index.name << "_remove_index(record_id);\n";

  out << "    " << tname << "_FK.free(record_id + 1);\n";
  out << "   }\n";
 }

 out << '\n';
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
  out << "   void internal_insert_" << tname << "(record_id_t record_id)\n";
  out << "   {\n";
  out << "    " << tname << "_FK.use(record_id + 1);\n";

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
    out << "    " << index.name << "_add_index(record_id);\n";

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
   out << "    " << tname << "_FK.get_record(record_id + 1)." << fname;
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

   const std::string &name = table.second.get_name();
   out << "if (table_id == " << table.first << ")\n";
   out << "     internal_delete_" << name << "(record_id);\n";
  }
 }
 out << "   }\n";

 //
 // after_insert listener function
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
   out << "     while (" << name << "_FK.size() < record_id)\n";
   out << "      " << name << "_FK.push_back();\n";
   out << "     internal_insert_" << name << "(record_id);\n";
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
 // Do nothing for model changes
 //
 out << R"RRR(
   void after_create_table(const std::string &name) override {}
   void after_drop_table(table_id_t table_id) override {}
   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        joedb::Type type) override
   {
   }
   void after_drop_field(table_id_t table_id, field_id_t field_id) override {}
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

  //
  // Declaration of container access
  //
  out << "   " << tname << "_container get_" << tname << "_table() const;\n\n";

  //
  // new with default fields
  //
  out << "   " << tname << "_t new_" << tname << "()\n";
  out << "   {\n";
  out << "    size_t free_record = " << tname << "_FK.get_free_record();\n";
  out << "    " << tname << "_t result(free_record - 1);\n\n";
  out << "    internal_insert_" << tname << "(result.id);\n\n";
  out << "    listener->after_insert(" << table.first << ", result.id);\n";
  out << "    return result;\n";
  out << "   }\n";
  out << "   void clear_" << tname << "_table();\n";
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
    out << fname;
   }

   out << '\n';
  }
  out << "   )\n";
  out << "   {\n";
  out << "    size_t free_record = " << tname << "_FK.get_free_record();\n";

  for (const auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();

   out << "    " << tname << "_FK.get_record(free_record).";
   out << fname << " = " << fname << ";\n";
  }

  out << "\n    " << tname << "_t result(free_record - 1);\n";
  out << "    internal_insert_" << tname << "(result.id);\n\n";

  out << "    listener->after_insert(" << table.first << ", result.id);\n";
  for (const auto &field: table.second.get_fields())
  {
   const std::string &fname = field.second.get_name();

   out << "    listener->after_update_";
   out << types[int(field.second.get_type().get_type_id())];
   out << '(' << table.first << ", result.id, " << field.first << ", ";
   out << fname;
   if (field.second.get_type().get_type_id() == Type::type_id_t::reference)
    out << ".id";
   out << ");\n";
  }

  out << "    return result;\n";
  out << "   }\n\n";

  //
  // Delete
  //
  out << "   void delete_" << tname << "(" << tname << "_t record)\n";
  out << "   {\n";
  out << "    internal_delete_" << tname << "(record.id);\n";
  out << "    listener->after_delete(" << table.first << ", record.id);\n";
  out << "   }\n";

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
   out << "    return " << tname;
   out << "_FK.get_record(record.id + 1)." << fname << ";\n";
   out << "   }\n";

   //
   // Setter
   //
   out << "   void set_" << fname;
   out << "(" << tname << "_t record, ";
   write_type(out, db, field.second.get_type(), true);
   out << fname << ")\n";
   out << "   {\n";
   out << "    assert(!record.is_null());\n";
   out << "    internal_update_" << tname << '_' << fname << "(record.id, ";
   out << fname << ");\n";
   out << "    listener->after_update_";
   out << types[int(field.second.get_type().get_type_id())];
   out << '(' << table.first << ", record.id, " << field.first << ", ";
   out << fname;
   if (field.second.get_type().get_type_id() == Type::type_id_t::reference)
    out << ".id";
   out << ");\n";
   out << "   }\n";
  }
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
   write_tuple_type(out, table, index);
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

  public:
   File_Database(const char *file_name, bool read_only = false):
    file(file_name,
         read_only ? joedb::File::mode_t::read_existing :
                     joedb::File::mode_t::write_existing),
    journal(file)
   {
    if (is_good())
     journal.replay_log(*this);
    else if (file.get_status() == joedb::File::status_t::failure && !read_only)
    {
     file.open(file_name, joedb::File::mode_t::create_new);
     if (file.get_status() == joedb::File::status_t::success)
     {
      journal.~Journal_File();
      new(&journal) joedb::Journal_File(file);
)RRR";

 for (auto &table: tables)
 {
  out << "      journal.after_create_table(\"" << table.second.get_name()
      << "\");\n";

  for (auto field: table.second.get_fields())
  {
   out << "      journal.after_add_field("
       << table.first << ", \""
       << field.second.get_name() << "\", ";
   switch (field.second.get_type().get_type_id())
   {
    case Type::type_id_t::null:
    break;
    case Type::type_id_t::string:
     out << "joedb::Type::string()";
    break;
    case Type::type_id_t::int32:
     out << "joedb::Type::int32()";
    break;
    case Type::type_id_t::int64:
     out << "joedb::Type::int64()";
    break;
    case Type::type_id_t::reference:
     out << "joedb::Type::reference("
         << field.second.get_type().get_table_id() << ")";
    break;
    case Type::type_id_t::boolean:
     out << "joedb::Type::boolean()";
    break;
    case Type::type_id_t::float32:
     out << "joedb::Type::float32()";
    break;
    case Type::type_id_t::float64:
     out << "joedb::Type::float64()";
    break;
   }
   out << ");\n";
  }
 }

 out << R"RRR(     }
    }
    set_listener(journal);
   }

   joedb::Journal_File::state_t get_journal_state() const
   {
    return journal.get_state();
   }

   void commit() {file.commit();}
   void checkpoint() {journal.checkpoint();}
   void safe_commit()
   {
    file.commit();
    journal.checkpoint();
    file.commit();
   }
   bool is_good() const
   {
    return file.get_status() == joedb::File::status_t::success &&
           journal.get_state() == joedb::Journal_File::state_t::no_error;
   }
 };

)RRR";

 //
 // Plain iteration over tables
 //
 for (auto &table: tables)
 {
  const std::string &tname = table.second.get_name();
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
  out << "     const joedb::Freedom_Keeper<" << tname << "_data> &fk;\n";
  out << "     size_t index;\n";
  out << "     iterator(const joedb::Freedom_Keeper<" << tname << "_data> &fk): fk(fk), index(0) {}\n";
  out << '\n';
  out << "    public:\n";
  out << "     bool operator!=(const iterator &i) const {return index != i.index;}\n";
  out << "     iterator &operator++() {index = fk.get_next(index); return *this;}\n";

  out << "     " << tname << "_t operator*() {return ";
  out << tname << "_t(index - 1);}\n";
  out << "   };\n";
  out << '\n';
  out << "   iterator begin() {return ++iterator(db." << tname << "_FK);}\n";
  out << "   iterator end() {return iterator(db." << tname << "_FK);}\n";
  out << "   bool is_empty() const {return db." << tname
      << "_FK.is_empty();}\n";
  out << "   size_t get_size() const {return db." << tname << "_FK.get_used_count();}\n";
  out << "   static " << tname << "_t get_at(size_t i) {return "
      << tname << "_t(i);}\n";
  out << "   bool is_valid_at(size_t i) {return db." << tname << "_FK.is_used(i + 1);}\n";
  out << " };\n";
  out << '\n';

  out << " inline " << tname << "_container Database::get_" << tname << "_table() const\n";
  out << " {\n";
  out << "  return " << tname << "_container(*this);\n";
  out << " }\n";
  out << '\n';

  out << " inline void Database::clear_" << tname << "_table()\n";
  out << " {\n";
  out << "  while (!get_" << tname << "_table().is_empty())\n";
  out << "   delete_" << tname << "(*get_" << tname << "_table().begin());\n";
  out << " }\n";
  out << '\n';
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
   write_index_type(out, table, index);
   out << "::const_iterator, ";
   write_index_type(out, table, index);
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
   write_tuple_type(out, table, index);
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
   write_index_type(out, table, index);
   out << "::const_iterator map_iterator;\n";
   out << "     iterator(";
   write_index_type(out, table, index);
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
 std::ifstream joedbi_file(argv[1]);
 if (!joedbi_file.good())
 {
  std::cerr << "Error: could not open " << argv[1] << '\n';
  return 1;
 }

 joedb::Database db;
 joedb::Interpreter interpreter(db);
 interpreter.main_loop(joedbi_file, std::cerr);

 //
 // Read file.joedbc
 //
 std::ifstream joedbc_file(argv[2]);
 if (!joedbc_file.good())
 {
  std::cerr << "Error: could not open " << argv[2] << '\n';
  return 1;
 }

 joedb::Compiler_Options compiler_options(db);

 if (!joedb::parse_compiler_options(joedbc_file, std::cerr, compiler_options))
 {
  std::cerr << "Error: could not parse compiler options\n";
  return 1;
 }

 //
 // Generate code
 //
 joedb::generate_code(std::cout, compiler_options);

 return 0;
}
