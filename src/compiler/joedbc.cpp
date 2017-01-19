#include "Database.h"
#include "File.h"
#include "Stream_File.h"
#include "Journal_File.h"
#include "Selective_Writeable.h"
#include "Readable_Multiplexer.h"
#include "Interpreter.h"
#include "Compiler_Options.h"
#include "Compiler_Options_io.h"
#include "type_io.h"
#include "c_wrapper.h"
#include "Dummy_Writeable.h"
#include "is_identifier.h"

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
  case Type::Type_Id::null:
   out << "void ";
  break;

  case Type::Type_Id::string:
   if (return_type)
    out << "const std::string &";
   else
    out << "std::string ";
  break;

  case Type::Type_Id::reference:
   out << "id_of_" << db.get_table_name(type.get_table_id()) << ' ';
  break;

  #define TYPE_MACRO(type, return_type, type_id, read, write)\
  case Type::Type_Id::type_id:\
   out << #type << ' ';\
  break;
  #define TYPE_MACRO_NO_STRING
  #define TYPE_MACRO_NO_REFERENCE
  #include "TYPE_MACRO.h"
  #undef TYPE_MACRO_NO_REFERENCE
  #undef TYPE_MACRO_NO_STRING
  #undef TYPE_MACRO
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
 out << "std::tuple<";
 for (size_t i = 0; i < index.field_ids.size(); i++)
 {
  if (i > 0)
   out << ", ";
  const Type &type = db.get_field_type(index.table_id, index.field_ids[i]);
  write_type(out, db, type, false);
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

 out << ", id_of_" << db.get_table_name(index.table_id) << ">";
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
#include <map>
#include <algorithm>
#include <sstream>

#include "joedb/File.h"
#include "joedb/Journal_File.h"
#include "joedb/Database.h"
#include "joedb/Freedom_Keeper.h"
#include "joedb/Exception.h"
#include "joedb/Stream_File.h"
#include "joedb/joedb_assert.h"

)RRR";

 out << "namespace " << options.get_namespace_name() << "\n{\n";
 out << " class Database;\n\n";

 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  out << " class container_of_" << tname << ";\n";
 }

 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  out << '\n';
  out << " class id_of_" << tname << "\n {\n";
  out << "  friend class Database;\n";
  out << "  friend class File_Database;\n";
  for (auto &friend_table: tables)
   if (friend_table.first != table.first)
    out << "  friend class id_of_" << friend_table.second << ";\n";
  out << "  friend class container_of_"  << tname << ";\n";
  out << "\n  private:\n";
  out << "   Record_Id id;\n";
  out << "\n  public:\n";
  out << "   explicit id_of_" << tname << "(Record_Id id): id(id) {}\n";
  out << "   id_of_" << tname << "(): id(0) {}\n";
  out << "   bool is_null() const {return id == 0;}\n";
  out << "   Record_Id get_id() const {return id;}\n";
  out << "   bool operator==(id_of_" << tname << " x) const {return id == x.id;}\n";
  out << "   bool operator<(id_of_" << tname << " x) const {return id < x.id;}\n";
  out << "   id_of_" << tname << " operator[](Record_Id i) const {return id_of_" << tname << "(id + i);}\n";
  out << " };\n";
 }

 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  const auto storage = options.get_table_options(table.first).storage;

  out << "\n struct data_of_" << tname;

  if (storage == Compiler_Options::Table_Storage::freedom_keeper)
   out << ": public joedb::EmptyRecord";

  out <<"\n {\n";

  if (storage == Compiler_Options::Table_Storage::freedom_keeper)
  {
   out << "  data_of_" << tname << "() {}\n";
   out << "  data_of_" << tname << "(bool f): joedb::EmptyRecord(f) {}\n";
  }

  for (const auto &field: db.get_fields(table.first))
  {
   out << "  ";
   write_type(out, db, db.get_field_type(table.first, field.first), false);
   out << "field_value_of_" << field.second << ";\n";
  }

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
   {
    out << "  ";
    write_index_type(out, db, index);
    out << "::iterator ";
    out << "iterator_over_" << index.name << ";\n";
   }

  out << " };\n\n";
 }

 for (auto &index: options.get_indices())
  if (!index.unique)
   out << " class range_of_" << index.name << ";\n";
 out << '\n';

 out << " class Database: public joedb::Writeable\n {\n";

 for (auto &table: tables)
 {
  out << "  friend class id_of_"  << table.second << ";\n";
  out << "  friend class container_of_"  << table.second << ";\n";
 }

 for (auto &index: options.get_indices())
  if (!index.unique)
   out << "  friend class range_of_" << index.name << ";\n";

 out << R"RRR(
  protected:
   virtual void error(const char *message)
   {
    throw joedb::Exception(message);
   }

   Record_Id max_record_id;

  public:
   void set_max_record_id(Record_Id record_id)
   {
    max_record_id = record_id;
   }

  protected:
)RRR";

 //
 // Vectors, and freedom keepers
 //
 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  const auto storage = options.get_table_options(table.first).storage;

  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "   bool is_valid_record_id_for_" << tname << "(Record_Id record_id) const {return storage_of_" << tname << ".is_used(record_id + 1);}\n";
    out << "   joedb::Freedom_Keeper<data_of_" << tname << ">";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "   bool is_valid_record_id_for_" << tname << "(Record_Id record_id) const {return record_id <= storage_of_" << tname << ".size() && record_id > 0;}\n";
    out << "   std::vector<data_of_" << tname << ">";
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
  const std::string &tname = db.get_table_name(index.table_id);

  out << "   ";
  write_index_type(out, db, index);
  out << " index_of_" << index.name << ";\n";

  out << "   void remove_index_of_" << index.name << "(Record_Id record_id)\n";
  out << "   {\n";
  out << "    auto &iterator = storage_of_" << tname;
  out << "[record_id - 1].iterator_over_" << index.name << ";\n";
  out << "    if (iterator != index_of_" << index.name << ".end())\n";
  out << "    {\n";
  out << "     index_of_" << index.name << ".erase(iterator);\n";
  out << "     iterator = index_of_" << index.name << ".end();\n";
  out << "    }\n";
  out << "   }\n";

  out << "   void add_index_of_" << index.name << "(Record_Id record_id)\n";
  out << "   {\n";
  out << "    " << "data_of_" << tname << " &data = storage_of_";
  out << tname << "[record_id - 1];\n";
  out << "    auto result = index_of_" << index.name;
  out << ".insert\n    (\n     ";
  write_index_type(out, db, index);
  out << "::value_type\n     (\n      ";
  write_tuple_type(out, db, index);
  out << '(';
  for (size_t i = 0; i < index.field_ids.size(); i++)
  {
   if (i > 0)
    out << ", ";
   out << "data.field_value_of_";
   out << db.get_field_name(index.table_id, index.field_ids[i]);
  }
  out << ')';
  out << ",\n      id_of_" << tname << "(record_id)\n     )\n    );\n";
  if (index.unique)
  {
   out << "    if (!result.second)\n";
   out << "     error(\"" << index.name << " unique index failure\");\n";
   out << "    data.iterator_over_" << index.name << " = result.first;\n";
  }
  else
   out << "    data.iterator_over_" << index.name << " = result;\n";
  out << "   }\n";
 }

 //
 // Internal data-modification functions
 //
 out << '\n';
 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  const auto storage = options.get_table_options(table.first).storage;
  const bool has_delete = storage == Compiler_Options::Table_Storage::freedom_keeper;

  if (has_delete)
  {
   out << "   void internal_delete_" << tname << "(Record_Id record_id)\n";
   out << "   {\n";
   out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record_id));\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == table.first)
     out << "    remove_index_of_" << index.name << "(record_id);\n";

   out << "    storage_of_" << tname << ".free(record_id + 1);\n";
   out << "   }\n";
  }
 }

 out << '\n';
 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  auto storage = options.get_table_options(table.first).storage;

  out << "   void internal_insert_" << tname << "(Record_Id record_id)\n";
  out << "   {\n";

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
   {
    out << "    data_of_" << tname << " &data = storage_of_";
    out << tname << "[record_id - 1];\n";
    out << "    data.iterator_over_" << index.name << " = ";
    out << "index_of_" << index.name << ".end();\n";
   }

  if (storage == Compiler_Options::Table_Storage::freedom_keeper)
   out << "    storage_of_" << tname << ".use(record_id + 1);\n";
  out << "   }\n";
 }

 out << '\n';
 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  for (auto &field: db.get_fields(table.first))
  {
   const std::string &fname = field.second;
   out << "   void internal_update_" << tname << "__" << fname;
   out << "\n   (\n    Record_Id record_id,\n    ";
   const Type &type = db.get_field_type(table.first, field.first);
   write_type(out, db, type, true);
   out << "field_value_of_" << fname << "\n   )\n";
   out << "   {\n";
   out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record_id));\n";
   out << "    storage_of_" << tname << "[record_id - 1].field_value_of_" << fname;
   out << " = field_value_of_" << fname << ";\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == table.first &&
        std::find(index.field_ids.begin(),
                  index.field_ids.end(),
                  field.first) != index.field_ids.end())
    {
     out << "    remove_index_of_" << index.name << "(record_id);\n";
     out << "    add_index_of_" << index.name << "(record_id);\n";
    }
   out << "   }\n";
  }
 }

 //
 // delete_from writeable function
 //
 out << '\n';
 out << "   void delete_from(Table_Id table_id, Record_Id record_id) override\n";
 out << "   {\n";
 {
  bool first = true;
  for (auto table: tables)
  {
   const std::string &tname = table.second;
   const auto storage = options.get_table_options(table.first).storage;

   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   out << "if (table_id == " << table.first << ")\n";
   if (storage == Compiler_Options::Table_Storage::vector)
   {
    out << "     error(\"Can't delete in vector storage of table ";
    out << tname << "\");\n";
   }
   else
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
 out << "    if (record_id <= 0 || (max_record_id && record_id > max_record_id))\n";
 out << "     error(\"insert_into: too big\");\n";
 {
  bool first = true;
  for (auto &table: tables)
  {
   const std::string &tname = table.second;
   const auto storage = options.get_table_options(table.first).storage;

   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   out << "if (table_id == " << table.first << ")\n";
   out << "    {\n";
   if (storage == Compiler_Options::Table_Storage::vector)
   {
    out << "     if (record_id != storage_of_" << tname << ".size() + 1)\n";
    out << "      error(\"Non-contiguous insert in vector storage of table ";
    out << tname << "\");\n";
   }
   else
   {
    out << "     if (is_valid_record_id_for_" << tname << "(record_id))\n";
    out << "      error(\"Duplicate insert into table " << tname << "\");\n";
   }
   out << "     if (storage_of_" << tname << ".size() < record_id)\n";
   out << "      storage_of_" << tname << ".resize(record_id);\n";
   out << "     internal_insert_" << tname << "(record_id);\n";
   out << "    }\n";
  }
 }
 out << "   }\n";

 //
 // insert_vector
 //
 out << '\n';
 out << "   void insert_vector(Table_Id table_id, Record_Id record_id, Record_Id size) override\n";
 out << "   {\n";
 out << "    if (record_id <= 0 || size <= 0 || (max_record_id && (record_id > max_record_id || size > max_record_id)))\n";
 out << "     error(\"insert_vector: too big\");\n";
 {
  bool first = true;
  for (auto &table: tables)
  {
   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   const std::string &name = table.second;
   out << "if (table_id == " << table.first << ")\n";
   out << "    {\n";
   out << "     if (storage_of_" << name << ".size() < record_id + size - 1)\n";
   out << "      storage_of_" << name << ".resize(record_id + size - 1);\n";
   out << "     for (Record_Id i = 0; i < size; i++)\n";
   out << "      internal_insert_" << name << "(record_id + i);\n";
   out << "    }\n";
  }
 }
 out << "   }\n";

 //
 // update
 //
 {
  for (int type_id = 1; type_id < int(Type::type_ids); type_id++)
  {
   out << '\n';
   out << "   void update_" << types[type_id] << '\n';
   out << "   (\n";
   out << "    Table_Id table_id,\n";
   out << "    Record_Id record_id,\n";
   out << "    Field_Id field_id,\n";
   out << "    " << cpp_types[type_id] << "value\n";
   out << "   )\n";
   out << "   override\n";
   out << "   {\n";

   for (auto &table: tables)
   {
    bool has_typed_field = false;

    for (auto &field: db.get_fields(table.first))
    {
     const Type &type = db.get_field_type(table.first, field.first);
     if (int(type.get_type_id()) == type_id)
     {
      has_typed_field = true;
      break;
     }
    }

    if (has_typed_field)
    {
     out << "    if (table_id == " << table.first << ")\n";
     out << "    {\n";

     for (auto &field: db.get_fields(table.first))
     {
      const Type &type = db.get_field_type(table.first, field.first);
      if (int(type.get_type_id()) == type_id)
      {
       out << "     if (field_id == " << field.first << ")\n";
       out << "     {\n";
       out << "      internal_update_" << table.second;
       out << "__" << field.second << "(record_id, ";
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
  for (int type_id = 1; type_id < int(Type::type_ids); type_id++)
  {
   out << '\n';
   out << "   void update_vector_" << types[type_id] << '\n';
   out << "   (\n";
   out << "    Table_Id table_id,\n";
   out << "    Record_Id record_id,\n";
   out << "    Field_Id field_id,\n";
   out << "    Record_Id size,\n";
   out << "    const " << storage_types[type_id] << " *value\n";
   out << "   )\n";
   out << "   override\n";
   out << "   {\n";

   for (auto &table: tables)
   {
    bool has_typed_field = false;

    for (auto &field: db.get_fields(table.first))
    {
     const Type &type = db.get_field_type(table.first, field.first);
     if (int(type.get_type_id()) == type_id)
     {
      has_typed_field = true;
      break;
     }
    }

    if (has_typed_field)
    {
     out << "    if (table_id == " << table.first << ")\n";
     out << "    {\n";

     for (auto &field: db.get_fields(table.first))
     {
      const Type &type = db.get_field_type(table.first, field.first);
      if (int(type.get_type_id()) == type_id)
      {
       out << "     if (field_id == " << field.first << ")\n";
       out << "     {\n";
       out << "      for (Record_Id i = 0; i < size; i++)\n";
       out << "       internal_update_" << table.second;
       out << "__" << field.second << "(record_id + i, ";
       if (type.get_type_id() != Type::Type_Id::reference)
        out << "value[i]";
       else
       {
        out << "id_of_" << db.get_table_name(type.get_table_id());
        out << "(value[i])";
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
 // Informative events are ignored
 //
 out << R"RRR(
   void comment(const std::string &comment) override {};
   void timestamp(int64_t timestamp) override {};
   void valid_data() override {};
)RRR";

 //
 // Schema changes are forwarded to the schema string
 //
 out << R"RRR(
  protected:
   static const std::string schema_string;
   bool upgrading_schema = false;
   std::stringstream schema_stream;
   joedb::Stream_File schema_file;
   joedb::Journal_File schema_journal;

   void check_schema()
   {
    schema_file.flush();

    const size_t file_schema_size = schema_stream.str().size();
    const size_t pos = joedb::Journal_File::header_size;
    const size_t len = file_schema_size - pos;

    if (schema_stream.str().compare(pos, len, schema_string, pos, len) != 0)
     throw joedb::Exception("Trying to open a file with incompatible schema");
   }

   void create_table(const std::string &name) override
   {
    schema_journal.create_table(name);
   }

   void drop_table(Table_Id table_id) override
   {
    schema_journal.drop_table(table_id);
   }

   void rename_table
   (
    Table_Id table_id,
    const std::string &name
   ) override
   {
    schema_journal.rename_table(table_id, name);
   }

   void add_field
   (
    Table_Id table_id,
    const std::string &name,
    joedb::Type type
   ) override
   {
    schema_journal.add_field(table_id, name, type);
   }

   void drop_field(Table_Id table_id, Field_Id field_id) override
   {
    schema_journal.drop_field(table_id, field_id);
   }

   void rename_field
   (
    Table_Id table_id,
    Field_Id field_id,
    const std::string &name
   ) override
   {
    schema_journal.rename_field(table_id, field_id, name);
   }

   void custom(const std::string &name) override
   {
    schema_journal.custom(name);
   }
)RRR";

 //
 // Public stuff
 //
 out << R"RRR(
  public:
   Database():
    max_record_id(0),
    schema_file(schema_stream, joedb::Open_Mode::create_new),
    schema_journal(schema_file)
   {}
)RRR";

 for (auto &table: tables)
 {
  out << '\n';
  const std::string &tname = table.second;
  const auto storage = options.get_table_options(table.first).storage;

  //
  // Declaration of container access
  //
  out << "   container_of_" << tname << " get_" << tname << "_table() const;\n\n";

  out << "   id_of_" << tname << " get_beginning_of_" << tname << "() const\n";
  out << "   {\n";
  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "    return id_of_" << tname << "(storage_of_" << tname << ".get_first_used() - 1);\n";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "    return id_of_" << tname << "(1);\n";
   break;
  }
  out << "   }\n";

  out << "   id_of_" << tname << " get_end_of_" << tname << "() const\n";
  out << "   {\n";
  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "    return id_of_" << tname << "(-1);\n";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "    return id_of_" << tname << "(storage_of_" << tname << ".size() + 1);\n";
   break;
  }
  out << "   }\n";

  out << "   id_of_" << tname << " get_next_" << tname << "(id_of_" << tname << " id) const\n";
  out << "   {\n";
  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "    return id_of_" << tname << "(storage_of_" << tname << ".get_next(id.get_id() + 1) - 1);\n";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "    return id_of_" << tname << "(id.get_id() + 1);\n";
   break;
  }
  out << "   }\n\n";


  out << "   template<class Comparator>\n";
  out << "   std::vector<id_of_" << tname << "> sorted_" << tname;
  out << "(Comparator comparator) const;\n\n";

  //
  // Easy access to null
  //
  out << "   id_of_" << tname << " null_" << tname << "() const\n";
  out << "   {\n";
  out << "    return id_of_" << tname << "();\n";
  out << "   }\n\n";


  //
  // Loop over fields
  //
  for (const auto &field: db.get_fields(table.first))
  {
   const std::string &fname = field.second;
   const Type &type = db.get_field_type(table.first, field.first);

   out << '\n';

   //
   // Getter
   //
   out << "   ";
   write_type(out, db, type, true);
   out << "get_" << fname << "(id_of_" << tname << " record) const\n";
   out << "   {\n";
   out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record.id));\n";
   out << "    return storage_of_" << tname;
   out << "[record.id - 1].field_value_of_" << fname << ";\n";
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
   out << "   id_of_" << tname << " find_" << index.name << '(';
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";
    const Type &type = db.get_field_type(index.table_id, index.field_ids[i]);
    write_type(out, db, type, true);
    out << "field_value_of_";
    out << db.get_field_name(index.table_id, index.field_ids[i]);
   }
   out << ") const\n";
   out << "   {\n";
   out << "    auto i = index_of_" << index.name << ".find(";
   write_tuple_type(out, db, index);
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
    write_type(out, db, type, true);
    out << "field_value_of_";
    out << db.get_field_name(index.table_id, index.field_ids[i]);
   }
   out << ") const;\n";
  }

 out << " };\n";

 //
 // File_Database
 //
 out << R"RRR(
 class Readonly_Database: public Database
 {
  private:
   joedb::File file;
   joedb::Readonly_Journal journal;

  public:
   Readonly_Database(const char *file_name):
    file(file_name, joedb::Open_Mode::read_existing),
    journal(file)
   {
    max_record_id = journal.get_checkpoint_position();
    journal.replay_log(*this);
    max_record_id = 0;

    check_schema();
   }
 };

 class File_Database: public Database
 {
  protected:
   void error(const char *message) override
   {
    if (ready_to_write)
    {
     write_timestamp();
     write_comment(message);
     checkpoint_no_commit();
    }
    Database::error(message);
   }

  private:
   joedb::File file;
   joedb::Journal_File journal;
   bool ready_to_write;

   void custom(const std::string &name) override
   {
    Database::custom(name);
)RRR";
 if (options.get_custom_names().size())
 {
  out << "    if (upgrading_schema)\n";
  out << "    {\n";
  for (const auto &name: options.get_custom_names())
  {
   out << "     if (name == \"" << name << "\")\n";
   out << "      " << name << "(*this);\n";
  }
  out << "    }\n";
 }
 out << "   }\n";

 if (options.get_custom_names().size())
 {
  out << '\n';
  for (const auto &name: options.get_custom_names())
   out << "   static void " << name << "(File_Database &db);\n";
 }

 out << R"RRR(
  public:
   File_Database(const char *file_name);

   uint64_t ahead_of_checkpoint() const {return journal.ahead_of_checkpoint();}
   void checkpoint_no_commit() {journal.checkpoint(0);}
   void checkpoint_half_commit() {journal.checkpoint(1);}
   void checkpoint_full_commit() {journal.checkpoint(2);}

   void write_comment(const std::string &comment);
   void write_timestamp();
   void write_timestamp(int64_t timestamp);
   void write_valid_data();
)RRR";

 for (auto &table: tables)
 {
  out << '\n';
  const std::string &tname = table.second;
  const auto storage = options.get_table_options(table.first).storage;
  const bool has_delete = storage == Compiler_Options::Table_Storage::freedom_keeper;

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
  out << "   id_of_" << tname << " new_" << tname << "()\n";
  out << "   {\n";

  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "    size_t free_record = storage_of_" << tname << ".get_free_record();\n";
    out << "    id_of_" << tname << " result(free_record - 1);\n\n";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "    const size_t size = storage_of_" << tname << ".size();\n";
    out << "    storage_of_" << tname << ".resize(size + 1);\n";
    out << "    id_of_" << tname << " result(size + 1);\n\n";
   break;
  }

  out << "    internal_insert_" << tname << "(result.id);\n\n";
  out << "    journal.insert_into(" << table.first << ", result.id);\n";
  out << "    return result;\n";
  out << "   }\n";
  out << '\n';

  //
  // new uninitialized vector
  //
  out << "   id_of_" << tname << " new_vector_of_" << tname << "(size_t size)\n";
  out << "   {\n";
  out << "    id_of_" << tname << " result(storage_of_" << tname;
  out << ".size() + 1);\n";
  out << "    storage_of_" << tname << ".resize(storage_of_";
  out << tname << ".size() + size);\n";
  out << "    for (size_t i = 0; i < size; i++)\n";
  out << "     internal_insert_" << tname << "(result.id + i);\n";
  out << "    journal.insert_vector(" << table.first;
  out << ", result.id, size);\n";
  out << "    return result;\n";
  out << "   }\n";
  out << '\n';

  //
  // new with all fields
  //
  out << "   id_of_" << tname << " new_" << tname << '\n';
  out << "   (\n    ";
  {
   bool first = true;

   for (const auto &field: db.get_fields(table.first))
   {
    const std::string &fname = field.second;

    if (first)
     first = false;
    else
     out << ",\n    ";

    const Type &type = db.get_field_type(table.first, field.first);
    write_type(out, db, type, true);
    out << "field_value_of_" << fname;
   }

   out << '\n';
  }
  out << "   )\n";
  out << "   {\n";
  out << "    auto result = new_" << tname << "();\n";

  for (const auto &field: db.get_fields(table.first))
  {
   const std::string &fname = field.second;
   out << "    set_" << fname << "(result, field_value_of_" << fname << ");\n";
  }

  out << "    return result;\n";
  out << "   }\n\n";

  //
  // Delete
  //
  if (has_delete)
  {
   out << "   void delete_" << tname << "(id_of_" << tname << " record)\n";
   out << "   {\n";
   out << "    internal_delete_" << tname << "(record.id);\n";
   out << "    journal.delete_from(" << table.first << ", record.id);\n";
   out << "   }\n";
  }

  //
  // Loop over fields
  //
  for (const auto &field: db.get_fields(table.first))
  {
   const std::string &fname = field.second;
   const Type &type = db.get_field_type(table.first, field.first);

   out << '\n';

   //
   // Setter
   //
   out << "   void set_" << fname;
   out << "(id_of_" << tname << " record, ";
   write_type(out, db, type, true);
   out << "field_value_of_" << fname << ")\n";
   out << "   {\n";
   out << "    internal_update_" << tname << "__" << fname << "(record.id, ";
   out << "field_value_of_" << fname << ");\n";
   out << "    journal.update_";
   out << types[int(type.get_type_id())];
   out << '(' << table.first << ", record.id, " << field.first << ", ";
   out << "field_value_of_" << fname;
   if (type.get_type_id() == Type::Type_Id::reference)
    out << ".id";
   out << ");\n";
   out << "   }\n";
  }
 }

 out << " };\n";

 //
 // Plain iteration over tables
 //
 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  const auto storage = options.get_table_options(table.first).storage;
  const bool has_delete = storage == Compiler_Options::Table_Storage::freedom_keeper;

  out << " class container_of_" << tname << "\n";
  out << " {\n";
  out << "  friend class Database;\n";
  out << '\n';
  out << "  private:\n";
  out << "   const Database &db;\n";
  out << "   container_of_" << tname << "(const Database &db): db(db) {}\n";
  out << '\n';
  out << "  public:\n";
  out << "   class iterator: public std::iterator<std::forward_iterator_tag,";
  out << " id_of_" << tname << ">\n";
  out << "   {\n";
  out << "    friend class container_of_" << tname << ";\n";
  out << "    private:\n";


  switch(storage)
  {
   case Compiler_Options::Table_Storage::freedom_keeper:
    out << "     const joedb::Freedom_Keeper<data_of_" << tname << "> &fk;\n";
    out << "     size_t index;\n";
    out << "     iterator(const joedb::Freedom_Keeper<data_of_" << tname << "> &fk): fk(fk), index(0) {}\n";
    out << "    public:\n";
    out << "     bool operator!=(const iterator &i) const {return index != i.index;}\n";
    out << "     iterator &operator++() {index = fk.get_next(index); return *this;}\n";
    out << "     id_of_" << tname << " operator*() {return id_of_";
    out << tname << "(index - 1);}\n";
    out << "   };\n";
    out << '\n';
    out << "   iterator begin() {return ++iterator(db.storage_of_" << tname << ");}\n";
    out << "   iterator end() {return iterator(db.storage_of_" << tname << ");}\n";
    out << "   bool is_empty() const {return db.storage_of_" << tname
        << ".is_empty();}\n";
    out << "   size_t get_size() const {return db.storage_of_" << tname << ".get_used_count();}\n";
    out << "   static id_of_" << tname << " get_at(size_t i) {return id_of_"
        << tname << "(i);}\n";
    out << "   bool is_valid_at(size_t i) {return db.storage_of_" << tname << ".is_used(i + 1);}\n";
   break;

   case Compiler_Options::Table_Storage::vector:
    out << "     size_t index;\n";
    out << "     iterator(size_t index): index(index) {}\n";
    out << "    public:\n";
    out << "     bool operator!=(const iterator &i) const {return index != i.index;}\n";
    out << "     iterator &operator++() {index++; return *this;}\n";
    out << "     id_of_" << tname << " operator*() {return id_of_" << tname << "(index + 1);}\n";
    out << "   };\n";
    out << '\n';
    out << "   iterator begin() {return iterator(0);}\n";
    out << "   iterator end() {return iterator(db.storage_of_" << tname << ".size());}\n";
    out << "   bool is_empty() const {return db.storage_of_" << tname << ".size() == 0;}\n";
    out << "   size_t get_size() const {return db.storage_of_" << tname << ".size();}\n";
    out << "   static id_of_" << tname << " get_at(size_t i) {return id_of_" << tname << "(i);}\n";
    out << "   bool is_valid_at(size_t i) {return i > 0 && i <= db.storage_of_" << tname << ".size();}\n";
   break;
  }


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
  out << "   result.push_back(x);\n";
  out << "  std::sort(result.begin(), result.end(), comparator);\n";
  out << "  return result;\n";
  out << " }\n";

  if (has_delete)
  {
   out << " inline void File_Database::clear_" << tname << "_table()\n";
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
   out << " class range_of_" << index.name << "\n";
   out << " {\n";
   out << "  friend class Database;\n";
   out << "  private:\n";
   out << "   std::pair<";
   write_index_type(out, db, index);
   out << "::const_iterator, ";
   write_index_type(out, db, index);
   out << "::const_iterator> range;\n";
   out << "   range_of_" << index.name << "(const Database &db";
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    out << ", ";
    const Type &type = db.get_field_type(index.table_id, index.field_ids[i]);
    write_type(out, db, type, true);
    out << db.get_field_name(index.table_id, index.field_ids[i]);
   }
   out << ")\n";
   out << "   {\n";
   out << "    range = db.index_of_" << index.name << ".equal_range(";
   write_tuple_type(out, db, index);
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
   out << "   class iterator: public std::iterator<std::forward_iterator_tag, ";
   out << "id_of_" << db.get_table_name(index.table_id) << ">\n";
   out << "   {\n";
   out << "    friend class range_of_" << index.name << ";\n";
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
   out << "     id_of_" << db.get_table_name(index.table_id);
   out << " operator*() const {return map_iterator->second;}\n";
   out << "   };\n";
   out << "   iterator begin() {return range.first;}\n";
   out << "   iterator end() {return range.second;}\n";
   out << " };\n";

   out << "   inline range_of_" << index.name << " Database::find_" << index.name << '(';
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    if (i > 0)
     out << ", ";
    const Type &type = db.get_field_type(index.table_id, index.field_ids[i]);
    write_type(out, db, type, true);
    out << "field_value_of_";
    out << db.get_field_name(index.table_id, index.field_ids[i]);
   }
   out << ") const\n";
   out << "   {\n";
   out << "    return range_of_" << index.name << "(*this";
   for (size_t i = 0; i < index.field_ids.size(); i++)
   {
    out << ", ";
    out << "field_value_of_";
    out << db.get_field_name(index.table_id, index.field_ids[i]);
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
 out << "#include \"joedb/Exception.h\"\n";
 out << '\n';
 out << "#include <sstream>\n";
 out << "#include <ctime>\n";
 out << '\n';
 out << "using namespace " << options.get_namespace_name() << ";\n";
 out << '\n';
 out << "const std::string Database::schema_string(";
 write_string(out, schema);
 out << ", ";
 out << schema.size();
 out << ");\n";

 out << R"RRR(
/////////////////////////////////////////////////////////////////////////////
void File_Database::write_comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 journal.comment(comment);
}

/////////////////////////////////////////////////////////////////////////////
void File_Database::write_timestamp()
/////////////////////////////////////////////////////////////////////////////
{
 journal.timestamp(std::time(0));
}

/////////////////////////////////////////////////////////////////////////////
void File_Database::write_timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 journal.timestamp(timestamp);
}

/////////////////////////////////////////////////////////////////////////////
void File_Database::write_valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 journal.valid_data();
}

/////////////////////////////////////////////////////////////////////////////
File_Database::File_Database(const char *file_name):
/////////////////////////////////////////////////////////////////////////////
 file(file_name, joedb::Open_Mode::write_existing_or_create_new),
 journal(file),
 ready_to_write(false)
{
 max_record_id = journal.get_checkpoint_position();
 journal.replay_log(*this);
 max_record_id = 0;

 ready_to_write = true;
 check_schema();

 const size_t file_schema_size = schema_stream.str().size();
 const size_t compiled_schema_size = schema_string.size();

 if (file_schema_size < compiled_schema_size)
 {
  journal.comment("Automatic schema upgrade");

  std::stringstream schema(schema_string);
  joedb::Stream_File schema_file(schema, joedb::Open_Mode::read_existing);
  joedb::Readonly_Journal schema_journal(schema_file);

  schema_journal.seek(file_schema_size);
  schema_journal.play_until_checkpoint(journal);

  schema_journal.seek(file_schema_size);
  upgrading_schema = true;
  schema_journal.play_until_checkpoint(*this);
  upgrading_schema = false;

  journal.comment("End of automatic schema upgrade");
 }
}
)RRR";
}

/////////////////////////////////////////////////////////////////////////////
void write_initial_comment(std::ostream &out, const Compiler_Options &options)
/////////////////////////////////////////////////////////////////////////////
{
 out << "/*///////////////////////////////////////////////////////////////////////////\n";
 out << "//\n";
 out << "// This code was automatically generated by the joedb compiler\n";
 out << "// https://www.remi-coulom.fr/joedb/\n";
 out << "//\n";
 out << "///////////////////////////////////////////////////////////////////////////*/\n";
}

/////////////////////////////////////////////////////////////////////////////
namespace joedb
/////////////////////////////////////////////////////////////////////////////
{
 class Custom_Collector: public Dummy_Writeable
 {
  private:
   std::vector<std::string> &names;

  public:
   Custom_Collector(std::vector<std::string> &names): names(names) {}

   void custom(const std::string &name) override
   {
    if (!is_identifier(name))
     throw Exception("custom: invalid identifier");
    names.push_back(name);
   }
 };
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
 std::vector<std::string> custom_names;

 {
  std::ifstream joedbi_file(argv[1]);
  if (!joedbi_file.good())
  {
   std::cerr << "Error: could not open " << argv[1] << '\n';
   return 1;
  }

  Stream_File schema_file(schema, Open_Mode::create_new);
  Journal_File journal(schema_file);
  Selective_Writeable schema_writeable(journal, Selective_Writeable::schema);
  Custom_Collector custom_collector(custom_names);

  Readable_Multiplexer multiplexer(db);
  multiplexer.add_writeable(schema_writeable);
  multiplexer.add_writeable(custom_collector);

  Interpreter interpreter(multiplexer);
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

 Compiler_Options compiler_options(db, custom_names);

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

 if (compiler_options.get_generate_c_wrapper())
 {
  std::ofstream header(compiler_options.get_namespace_name() + "_wrapper.h");
  std::ofstream body(compiler_options.get_namespace_name() + "_wrapper.cpp");
  write_initial_comment(header, compiler_options);
  write_initial_comment(body, compiler_options);
  joedb::generate_c_wrapper(header, body, compiler_options);
 }

 return 0;
}
