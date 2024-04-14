#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/get_time_string.h"
#include "joedb/io/type_io.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/compiler/Compiler_Options.h"
#include "joedb/compiler/Compiler_Options_io.h"
#include "joedb/compiler/c_wrapper.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/is_identifier.h"
#include "joedb/get_version.h"

#include <fstream>
#include <set>
#include <algorithm>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
// type arrays
/////////////////////////////////////////////////////////////////////////////
#define STRINGIFY(X) #X
#define EXPAND_AND_STRINGIFY(X) STRINGIFY(X)

static char const * const types[] =
{
 nullptr,
#define TYPE_MACRO(a, b, type_id, d, e) EXPAND_AND_STRINGIFY(type_id),
#include "joedb/TYPE_MACRO.h"
};

static char const * const cpp_types[] =
{
 nullptr,
#define TYPE_MACRO(a, type, c, d, e) EXPAND_AND_STRINGIFY(type)" ",
#include "joedb/TYPE_MACRO.h"
};

static char const * const storage_types[] =
{
 nullptr,
#define TYPE_MACRO(storage, b, c, d, e) EXPAND_AND_STRINGIFY(storage),
#include "joedb/TYPE_MACRO.h"
};

#undef EXPAND_AND_STRINGIFY
#undef STRINGIFY

/////////////////////////////////////////////////////////////////////////////
static void write_type
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Database &db,
 Type type,
 bool return_type,
 bool setter_type
)
{
 switch (type.get_type_id())
 {
  case Type::Type_Id::null:
   out << "void ";
  break;

  case Type::Type_Id::reference:
   out << "id_of_" << db.get_table_name(type.get_table_id()) << ' ';
  break;

  #define TYPE_MACRO(storage_tt, return_tt, type_id, read, write)\
  case Type::Type_Id::type_id:\
   if (return_type || setter_type)\
    out << #return_tt << ' ';\
   else\
    out << #storage_tt << ' ';\
  break;
  #define TYPE_MACRO_NO_REFERENCE
  #include "joedb/TYPE_MACRO.h"
 }
}

/////////////////////////////////////////////////////////////////////////////
static const char *get_type_name(Type type)
/////////////////////////////////////////////////////////////////////////////
{
 #define TYPE_MACRO(a, b, type_id, r, w) #type_id,
 static const char * const type_string[] =
 {
  "null",
  #include "joedb/TYPE_MACRO.h"
 };

 return type_string[int(type.get_type_id())];
}

/////////////////////////////////////////////////////////////////////////////
static void write_tuple_type
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
  write_type(out, db, type, false, false);
 }
 out << ">";
}

/////////////////////////////////////////////////////////////////////////////
static void write_index_type
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
static void generate_h(std::ostream &out, const Compiler_Options &options)
/////////////////////////////////////////////////////////////////////////////
{
 const Database &db = options.get_db();
 auto tables = db.get_tables();

 namespace_include_guard(out, "Database", options.get_name_space());

 out << '\n';
 out << "#include \"" << options.get_name_space().back() << "_readonly.h\"\n";
 out << "#include \"joedb/concurrency/Client.h\"\n";
 out << "#include \"joedb/concurrency/Local_Connection.h\"\n";
 out << "#include \"joedb/Span.h\"\n";
 out << '\n';

 namespace_open(out, options.get_name_space());

 //
 // Database
 //
 out << R"RRR(
 class Client_Data;
 class Client;

 class Generic_File_Database: public Database, public joedb::Blob_Reader
 {
  friend class Client_Data;
  friend class Client;

  protected:
   void error(const char *message) override
   {
    if (ready_to_write)
    {
     write_timestamp();
     write_comment(message);
     journal.flush();
    }
    Database::error(message);
   }

  private:
   joedb::Writable_Journal journal;
   bool ready_to_write;

   void initialize();
   void auto_upgrade();

   void custom(const std::string &name) final
   {
    Database::custom(name);
)RRR";

 if (!options.get_custom_names().empty())
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

 if (!options.get_custom_names().empty())
 {
  out << '\n';
  out << "  public:\n";
  for (const auto &name: options.get_custom_names())
   out << "   static void " << name << "(Generic_File_Database &db);\n";
  out << "\n  private:";
 }
 out << R"RRR(
   Generic_File_Database
   (
    joedb::Generic_File &file,
    bool upgrade,
    joedb::Commit_Level commit_level
   );

  public:
   Generic_File_Database
   (
    joedb::Generic_File &file,
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

 for (auto &table: tables)
 {
  out << '\n';
  const std::string &tname = table.second;

  //
  // Erase all elements of the table
  //
  out << "   void clear_" << tname << "_table();\n";
  out << '\n';

  //
  // Uninitialized new
  //
  out << "   id_of_" << tname << " new_" << tname << "()\n";
  out << "   {\n";

  out << "    id_of_" << tname << " result(Record_Id(storage_of_" << tname << ".freedom_keeper.get_free_record() - 1));\n";
  out << "    storage_of_" << tname << ".resize(storage_of_" << tname << ".freedom_keeper.size());\n";
  out << "    internal_insert_" << tname << "(result.get_record_id());\n\n";
  out << "    journal.insert_into(Table_Id(" << table.first << "), result.get_record_id());\n";
  out << "    return result;\n";
  out << "   }\n";
  out << '\n';

  //
  // new uninitialized vector
  //
  out << "   id_of_" << tname << " new_vector_of_" << tname << "(size_t size)\n";
  out << "   {\n";
  out << "    id_of_" << tname << " result(Record_Id(storage_of_" << tname;
  out << ".size() + 1));\n";
  out << "    storage_of_" << tname << ".resize(storage_of_";
  out << tname << ".size() + size);\n";
  out << "    internal_vector_insert_" << tname << "(result.get_record_id(), size);\n";
  out << "    journal.insert_vector(Table_Id(" << table.first;
  out << "), result.get_record_id(), size);\n";
  out << "    return result;\n";
  out << "   }\n";
  out << '\n';

  //
  // new with all fields
  //
  if (!db.get_fields(table.first).empty())
  {
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
     write_type(out, db, type, false, true);
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
  }

  //
  // Delete
  //
  out << "   void delete_" << tname << "(id_of_" << tname << " record)\n";
  out << "   {\n";
  out << "    internal_delete_" << tname << "(record.get_record_id());\n";
  out << "    journal.delete_from(Table_Id(" << table.first << "), record.get_record_id());\n";
  out << "   }\n\n";

  //
  // Loop over fields
  //
  for (const auto &field: db.get_fields(table.first))
  {
   const std::string &fname = field.second;
   const Type &type = db.get_field_type(table.first, field.first);

   //
   // Setter
   //
   out << "   void set_" << fname;
   out << "(id_of_" << tname << " record, ";
   write_type(out, db, type, false, true);
   out << "field_value_of_" << fname << ")\n";
   out << "   {\n";
   out << "    internal_update_" << tname << "__" << fname;

   out <<  "(record.get_record_id(), ";
   out << "field_value_of_" << fname << ");\n";
   out << "    journal.update_";
   out << types[int(type.get_type_id())];
   out << "(Table_Id(" << table.first << "), record.get_record_id(), Field_Id(" << field.first << "), ";
   out << "field_value_of_" << fname;
   if (type.get_type_id() == Type::Type_Id::reference)
    out << ".get_record_id()";
   out << ");\n";

   out << "   }\n\n";

   //
   // Vector update
   //
   out << "   template<typename F> void update_vector_of_" << fname;
   out << "(id_of_" << tname << " record, size_t size, F f)\n";
   out << "   {\n";
   out << "    std::exception_ptr exception;\n";
   out << "    joedb::Span<";
   write_type(out, db, type, false, false);
   out << "> span(&storage_of_" << tname;
   out << ".field_value_of_" << fname << "[record.get_id() - 1], size);\n";
   out << "    try {f(span);}\n";
   out << "    catch (...) {exception = std::current_exception();}\n";
   out << "    internal_update_vector_" << tname << "__" << fname << "(record.get_record_id(), size, span.begin());\n";
   out << "    journal.update_vector_" << types[int(type.get_type_id())] << "(Table_Id(" << table.first << "), record.get_record_id(), Field_Id(" << field.first << "), size, ";

   if (type.get_type_id() == Type::Type_Id::reference)
    out << "reinterpret_cast<Record_Id *>";

   out << "(span.begin()));\n";
   out << "    if (exception)\n";
   out << "     std::rethrow_exception(exception);\n";
   out << "   }\n\n";
  }
 }

 out << " };\n";

 out << R"RRR(
 class File_Database_Parent
 {
  public:
   joedb::File file;

   File_Database_Parent(const char *file_name, joedb::Open_Mode mode):
    file(file_name, mode)
   {
   }
 };

 class File_Database:
  public File_Database_Parent,
  public Generic_File_Database
 {
  public:
   File_Database
   (
    const char *file_name,
    joedb::Open_Mode mode = joedb::Open_Mode::write_existing_or_create_new,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    File_Database_Parent(file_name, mode),
    Generic_File_Database(file, commit_level)
   {
   }

   File_Database
   (
    const std::string &file_name,
    joedb::Open_Mode mode = joedb::Open_Mode::write_existing_or_create_new,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    File_Database(file_name.c_str(), mode)
   {
   }
 };

)RRR";

 for (auto &table: tables)
 {
  const std::string &tname = table.second;

  out << " inline void Generic_File_Database::clear_" << tname << "_table()\n";
  out << " {\n";
  out << "  while (!get_" << tname << "_table().is_empty())\n";
  out << "   delete_" << tname << "(get_" << tname << "_table().last());\n";
  out << " }\n";
  out << '\n';
 }

 //
 // Concurrency
 //
 out << R"RRR(
 ////////////////////////////////////////////////////////////////////////////
 class Client_Data: public joedb::Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Client;

  private:
   Generic_File_Database db;

  public:
   Client_Data
   (
    joedb::Generic_File &file,
    joedb::Commit_Level commit_level
   ):
    db(file, false, commit_level)
   {
   }

   bool is_readonly() const final
   {
    return false;
   }

   joedb::Writable_Journal &get_writable_journal() final
   {
    return db.journal;
   }

   void update() final
   {
    db.ready_to_write = false;
    db.journal.play_until_checkpoint(db);
    db.ready_to_write = true;
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Client: private Client_Data, public joedb::Client
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int64_t schema_checkpoint;

   void throw_if_schema_changed()
   {
    if (db.schema_journal.get_checkpoint_position() > schema_checkpoint)
     throw joedb::Exception("Can't upgrade schema during pull");
   }

  public:
   Client
   (
    joedb::Connection &connection,
    joedb::Generic_File &file,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    Client_Data(file, commit_level),
    joedb::Client(*this, connection)
   {
    if (get_checkpoint_difference() > 0)
     push_unlock();

    joedb::Client::transaction([this](joedb::Client_Data &data){
     db.check_schema();
     db.auto_upgrade();
    });

    schema_checkpoint = db.schema_journal.get_checkpoint_position();
   }

   const Database &get_database() const
   {
    return db;
   }

   int64_t pull()
   {
    const int64_t result = joedb::Client::pull();
    throw_if_schema_changed();
    return result;
   }

   template<typename F> void transaction(F transaction)
   {
    joedb::Client::transaction([&](joedb::Client_Data &data)
    {
     throw_if_schema_changed();
     transaction(db);
    });
   }
 };

#ifdef JOEDB_FILE_IS_LOCKABLE
 ////////////////////////////////////////////////////////////////////////////
 class Local_Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   joedb::File file;
   joedb::Local_Connection connection;

   Local_Client_Data(const char *file_name):
    file(file_name, joedb::Open_Mode::shared_write)
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Local_Client: private Local_Client_Data, public Client
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Local_Client(const char *file_name):
    Local_Client_Data(file_name),
    Client(Local_Client_Data::connection, Local_Client_Data::file)
   {
   }

   Local_Client(const std::string &file_name):
    Local_Client(file_name.c_str())
   {
   }
 };
#endif
)RRR";

 //
 // Types class
 //
 out << R"RRR(
 ////////////////////////////////////////////////////////////////////////////
 class Types: public Readonly_Types
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
)RRR";

 const std::vector<std::string> type_names
 {
  "File_Database",
  "Generic_File_Database",
  "Client"
 };

 for (const std::string &type_name: type_names)
 {
  out << "   typedef " << namespace_string(options.get_name_space());
  out << "::" << type_name << ' ' << type_name << ";\n";
 }

 out << " };\n";

 namespace_close(out, options.get_name_space());

 out << "\n#endif\n";
}

/////////////////////////////////////////////////////////////////////////////
static void generate_readonly_h
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options
)
{
 const Database &db = options.get_db();
 auto tables = db.get_tables();

 namespace_include_guard(out, "readonly_Database", options.get_name_space());

 out << R"RRR(
#include "joedb/Freedom_Keeper.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/Exception.h"
#include "joedb/exception/Out_Of_Date.h"
#include "joedb/assert.h"
#include "joedb/io/type_io.h"

#include <string>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

)RRR";

 namespace_open(out, options.get_name_space());

 out << R"RRR( using joedb::Record_Id;
 using joedb::Table_Id;
 using joedb::Field_Id;

 extern const char * schema_string;
 extern const size_t schema_string_size;
)RRR";

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
  out << "   constexpr id_of_" << tname << " operator[](size_t i) const {return id_of_" << tname << "(id + i);}\n";
  out << " };\n";
 }

 for (auto &table: tables)
 {
  const std::string &tname = table.second;

  out << "\n struct data_of_" << tname;

  out <<"\n {\n";

  std::vector<std::string> fields;

  for (const auto &field: db.get_fields(table.first))
  {
   fields.emplace_back("field_value_of_" + field.second);

   const joedb::Type &type = db.get_field_type(table.first, field.first);

   out << "  std::vector<";
   write_type(out, db, type, false, false);
   out << "> " << fields.back() << ";\n";
  }

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
   {
    out << "  std::vector<";
    write_index_type(out, db, index);
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

 for (auto &table: tables)
 {
  out << "  friend class id_of_"  << table.second << ";\n";
  out << "  friend class container_of_"  << table.second << ";\n";
 }

 for (const auto &index: options.get_indices())
  if (!index.unique)
   out << "  friend class range_of_" << index.name << ";\n";

 out << R"RRR(
  protected:
   virtual void error(const char *message)
   {
    throw joedb::Exception(message);
   }

   size_t max_record_id;

  public:
   void set_max_record_id(size_t record_id)
   {
    max_record_id = record_id;
   }

)RRR";

 //
 // Validity checks
 //
 for (auto &table: tables)
 {
  const std::string &tname = table.second;

  out << "   bool is_valid(id_of_" << tname << " id) const {return is_valid_record_id_for_" << tname << "(id.get_record_id());}\n";
 }

 out << "\n  protected:\n";

 for (auto &table: tables)
 {
  const std::string &tname = table.second;

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
  write_index_type(out, db, index);
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
  write_index_type(out, db, index);
  out << "::value_type\n     (\n      ";
  write_tuple_type(out, db, index);
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
    out << "     joedb::write_" << get_type_name(type) << "(out, ";
    out << "storage_of_" << tname << ".field_value_of_";
    out << db.get_field_name(index.table_id, index.field_ids[i]);
    out << "[size_t(record_id) - 1]";
    if (type.get_type_id() == joedb::Type::Type_Id::reference)
     out << ".get_record_id()";
    out << ");\n";
   }
   out << "     out << \") at id = \" << record_id << ' ';\n";
   out << "     out << \"was already at id = \" << result.first->second.get_id();\n";
   out << "     error(out.str().c_str());\n";
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
 for (auto &table: tables)
 {
  const std::string &tname = table.second;

  out << "   void internal_delete_" << tname << "(Record_Id record_id)\n";
  out << "   {\n";
  out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record_id));\n";

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
    out << "    remove_index_of_" << index.name << "(record_id);\n";

  out << "    storage_of_" << tname << ".freedom_keeper.free(size_t(record_id) + 1);\n";
  out << "   }\n";
 }

 out << '\n';
 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  const auto table_options = options.get_table_options(table.first);
  const auto null_initialization = table_options.null_initialization;

  out << "   void internal_insert_" << tname << "(Record_Id record_id)\n";
  out << "   {\n";

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
   {
    out << "    storage_of_" << tname;
    out << ".iterator_over_" << index.name << "[size_t(record_id) - 1] = ";
    out << "index_of_" << index.name << ".end();\n";
   }

  out << "    storage_of_" << tname << ".freedom_keeper.use(size_t(record_id) + 1);\n";

  if (null_initialization)
   for (const auto &field: db.get_fields(table.first))
   {
    const std::string &fname = field.second;
    const Type &type = db.get_field_type(table.first, field.first);

    out << "    storage_of_" << tname << ".field_value_of_";
    out << fname << "[size_t(record_id) - 1]";

    if (type.get_type_id() == Type::Type_Id::string)
    {
     out << ".clear()";
    }
    else if (type.get_type_id() == Type::Type_Id::reference)
    {
     out << " = ";
     write_type(out, db, type, false, false);
     out << "(Record_Id(0))";
    }
    else
    {
     out << " = 0";
    }

    out << ";\n";
   }

  out << "   }\n\n";

  out << "   void internal_vector_insert_" << tname << "(Record_Id record_id, size_t size)\n";
  out << "   {\n";
  out << "    storage_of_" << tname << ".freedom_keeper.use_vector(size_t(record_id) + 1, size);\n";

  for (const auto &index: options.get_indices())
   if (index.table_id == table.first)
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
 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  for (const auto &field: db.get_fields(table.first))
  {
   const std::string &fname = field.second;
   const Type &type = db.get_field_type(table.first, field.first);

   out << "   void internal_update_" << tname << "__" << fname;
   out << "\n   (\n    Record_Id record_id,\n    ";
   write_type(out, db, type, true, false);
   out << "field_value_of_" << fname << "\n   )\n";
   out << "   {\n";
   out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record_id));\n";
   out << "    storage_of_" << tname << ".field_value_of_" << fname;
   out << "[size_t(record_id) - 1] = field_value_of_" << fname;
   out << ";\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == table.first &&
        std::find(index.field_ids.begin(),
                  index.field_ids.end(),
                  field.first) != index.field_ids.end())
    {
     out << "    remove_index_of_" << index.name << "(record_id);\n";
     out << "    add_index_of_" << index.name << "(record_id);\n";
    }
   out << "   }\n\n";

   out << "   void internal_update_vector_" << tname << "__" << fname << '\n';
   out << "   (\n";
   out << "    Record_Id record_id,\n";
   out << "    size_t size,\n";
   out << "    const ";
   write_type(out, db, type, false, false);
   out << " *value\n";
   out << "   )\n";
   out << "   {\n";
   out << "    for (size_t i = 0; i < size; i++)\n";
   out << "     JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record_id + i));\n";
   out << "    ";
   write_type(out, db, type, false, false);
   out << " *target = &storage_of_" << tname;
   out << ".field_value_of_" << fname << "[size_t(record_id) - 1];\n";
   out << "    if (target != value)\n";
   out << "     std::copy_n(value, size, target);\n";

   for (const auto &index: options.get_indices())
    if (index.table_id == table.first &&
        std::find(index.field_ids.begin(),
                  index.field_ids.end(),
                  field.first) != index.field_ids.end())
    {
     out << "    for (size_t i = 0; i < size; i++)\n";
     out << "     remove_index_of_" << index.name << "(record_id + i);\n";
     out << "    for (size_t i = 0; i < size; i++)\n";
     out << "     add_index_of_" << index.name << "(record_id + i);\n";
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
  for (auto &table: tables)
  {
   const std::string &tname = table.second;

   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   out << "if (table_id == Table_Id(" << table.first << "))\n";
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
 out << "     error(\"insert_into: too big\");\n";
 {
  bool first = true;
  for (auto &table: tables)
  {
   const std::string &name = table.second;

   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   out << "if (table_id == Table_Id(" << table.first << "))\n";
   out << "    {\n";
   out << "     if (is_valid_record_id_for_" << name << "(record_id))\n";
   out << "      error(\"Duplicate insert into table " << name << "\");\n";
   out << "     if (storage_of_" << name << ".size() < size_t(record_id))\n";
   out << "      storage_of_" << name << ".resize(size_t(record_id));\n";
   out << "     internal_insert_" << name << "(record_id);\n";
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
     error("insert_vector: null record_id, or too big");
    }
)RRR";

 {
  bool first = true;
  for (auto &table: tables)
  {
   const std::string &name = table.second;

   out << "    ";
   if (first)
    first = false;
   else
    out << "else ";

   out << "if (table_id == Table_Id(" << table.first << "))\n";
   out << "    {\n";
   out << "     if (storage_of_" << name << ".size() < size_t(record_id) + size - 1)\n";
   out << "      storage_of_" << name << ".resize(size_t(record_id) + size - 1);\n";
   out << "     internal_vector_insert_" << name << "(record_id, size);\n";
   out << "    }\n";
  }
 }
 out << "   }\n";

 //
 // set of existing types in the database
 //
 std::set<Type::Type_Id> db_types;

 for (auto &table: tables)
  for (const auto &field: db.get_fields(table.first))
   db_types.insert(db.get_field_type(table.first, field.first).get_type_id());

 //
 // update
 //
 {
  for (int type_id = 1; type_id < int(Type::type_ids); type_id++)
  {
   if (db_types.find(Type::Type_Id(type_id)) == db_types.end())
    continue;

   out << '\n';
   out << "   void update_" << types[type_id] << '\n';
   out << "   (\n";
   out << "    Table_Id table_id,\n";
   out << "    Record_Id record_id,\n";
   out << "    Field_Id field_id,\n";
   out << "    " << cpp_types[type_id] << "value\n";
   out << "   )\n";
   out << "   final\n";
   out << "   {\n";

   for (auto &table: tables)
   {
    bool has_typed_field = false;

    for (const auto &field: db.get_fields(table.first))
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
     out << "    if (table_id == Table_Id(" << table.first << "))\n";
     out << "    {\n";

     for (const auto &field: db.get_fields(table.first))
     {
      const Type &type = db.get_field_type(table.first, field.first);
      if (int(type.get_type_id()) == type_id)
      {
       out << "     if (field_id == Field_Id(" << field.first << "))\n";
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
   if (db_types.find(Type::Type_Id(type_id)) == db_types.end())
    continue;

   out << '\n';
   out << "   void update_vector_" << types[type_id] << '\n';
   out << "   (\n";
   out << "    Table_Id table_id,\n";
   out << "    Record_Id record_id,\n";
   out << "    Field_Id field_id,\n";
   out << "    size_t size,\n";
   out << "    const " << storage_types[type_id] << " *value\n";
   out << "   )\n";
   out << "   final\n";
   out << "   {\n";

   for (auto &table: tables)
   {
    bool has_typed_field = false;

    for (const auto &field: db.get_fields(table.first))
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
     out << "    if (table_id == Table_Id(" << table.first << "))\n";
     out << "    {\n";

     for (const auto &field: db.get_fields(table.first))
     {
      const Type &type = db.get_field_type(table.first, field.first);
      if (int(type.get_type_id()) == type_id)
      {
       out << "     if (field_id == Field_Id(" << field.first << "))\n";
       out << "     {\n";
       out << "      internal_update_vector_" << table.second;
       out << "__" << field.second << "(record_id, size, ";

       if (type_id != int(joedb::Type::Type_Id::reference))
        out << "value";
       else
       {
        out << "reinterpret_cast<const ";
        write_type(out, db, type, false, false);
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
  for (int type_id = 1; type_id < int(Type::type_ids); type_id++)
  {
   if (db_types.find(Type::Type_Id(type_id)) == db_types.end())
    continue;

   out << '\n';
   out << "   " << storage_types[type_id];
   out << " *get_own_" << types[type_id] << "_storage\n";
   out << "   (\n";
   out << "    Table_Id table_id,\n";
   out << "    Record_Id record_id,\n";
   out << "    Field_Id field_id,\n";
   out << "    size_t &capacity\n";
   out << "   )\n";
   out << "   final\n";
   out << "   {\n";

   for (auto &table: tables)
   {
    bool has_typed_field = false;

    for (const auto &field: db.get_fields(table.first))
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
     out << "    if (table_id == Table_Id(" << table.first << "))\n";
     out << "    {\n";
     out << "     capacity = size_t(storage_of_" << table.second << ".freedom_keeper.size());\n";

     for (const auto &field: db.get_fields(table.first))
     {
      const Type &type = db.get_field_type(table.first, field.first);
      if (int(type.get_type_id()) == type_id)
      {
       out << "     if (field_id == Field_Id(" << field.first << "))\n"
           << "     {\n"
           << "      return ";

       if (type_id == int(Type::Type_Id::reference))
        out << "reinterpret_cast<Record_Id *>";

       out << "(storage_of_" << table.second;
       out << ".field_value_of_" << field.second << ".data() + size_t(record_id) - 1);\n"
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
     throw joedb::Exception("Trying to open a file with incompatible schema");
   }

   void create_table(const std::string &name) final
   {
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
   ) final
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

   void initialize_with_readonly_journal(joedb::Readonly_Journal &journal)
   {
    max_record_id = size_t(journal.get_checkpoint_position());
    journal.replay_log(*this);
    max_record_id = 0;

    check_schema();

    if (requires_schema_upgrade())
     throw joedb::exception::Out_Of_Date();
   }
)RRR";

 for (auto &table: tables)
 {
  out << '\n';
  const std::string &tname = table.second;

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
   write_type(out, db, type, true, false);
   out << "get_" << fname << "(id_of_" << tname << " record) const\n";
   out << "   {\n";
   out << "    JOEDB_ASSERT(is_valid_record_id_for_" << tname << "(record.get_record_id()));\n";
   out << "    return (";
   write_type(out, db, type, true, false);
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
    write_type(out, db, type, true, false);
    out << "field_value_of_";
    out << db.get_field_name(index.table_id, index.field_ids[i]);
   }
   out << ") const\n";
   out << "   {\n";
   out << "    const auto i = index_of_" << index.name << ".find(";
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
    write_type(out, db, type, true, false);
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
  public:
   Readonly_Database(joedb::Readonly_Journal &journal)
   {
    initialize_with_readonly_journal(journal);
   }

   Readonly_Database(joedb::Readonly_Journal &&journal):
    Readonly_Database(journal)
   {
   }

   Readonly_Database(joedb::Generic_File &file):
    Readonly_Database(joedb::Readonly_Journal(file))
   {
   }

   Readonly_Database(joedb::Generic_File &&file):
    Readonly_Database(file)
   {
   }

   Readonly_Database(const char *file_name):
    Readonly_Database
    (
     joedb::File(file_name, joedb::Open_Mode::read_existing)
    )
   {
   }

   Readonly_Database(const std::string &file_name):
    Readonly_Database(file_name.c_str())
   {
   }
 };

 class Pullable_Database: public Database
 {
  private:
   joedb::Readonly_Journal journal;

  public:
   Pullable_Database(joedb::File &file): journal(file)
   {
    initialize_with_readonly_journal(journal);
   }

   bool pull()
   {
    journal.refresh_checkpoint();
    if (journal.get_position() < journal.get_checkpoint_position())
    {
     journal.play_until_checkpoint(*this);
     return true;
    }
    else
     return false;
   }
 };

 typedef Readonly_Database Generic_Readonly_Database;

)RRR";

 //
 // Plain iteration over tables
 //
 for (auto &table: tables)
 {
  const std::string &tname = table.second;

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
    write_type(out, db, type, true, false);
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
   out << "   class iterator\n";
   out << "   {\n";
   out << "    friend class range_of_" << index.name << ";\n";
   out << "    private:\n";
   out << "     ";
   write_index_type(out, db, index);
   out << "::const_iterator map_iterator;\n";
   out << "     iterator(";
   write_index_type(out, db, index);
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
    write_type(out, db, type, true, false);
    out << "field_value_of_";
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

 //
 // Types class
 //
out << R"RRR(
 class Interpreted_File;
 class Interpreted_Database;

 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Types
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
)RRR";

 std::vector<std::string> type_names
 {
  "Database",
  "Readonly_Database",
  "Interpreted_File",
  "Interpreted_Database"
 };

 for (auto &table: tables)
 {
  const std::string &tname = table.second;
  type_names.emplace_back("id_of_" + tname);

  out << "   typedef " << namespace_string(options.get_name_space());
  out << "::container_of_";
  out << tname << "::iterator iterator_of_" << tname << ";\n";
 }

 for (const std::string &type_name: type_names)
 {
  out << "   typedef " << namespace_string(options.get_name_space());
  out << "::" << type_name << ' ' << type_name << ";\n";
 }

 out << " };\n";

 namespace_close(out, options.get_name_space());

 out << "\n#endif\n";
}

/////////////////////////////////////////////////////////////////////////////
static void generate_readonly_cpp
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options,
 const std::vector<char> &schema
)
{
 const std::vector<std::string> &ns = options.get_name_space();
 out << "#include \"" << ns.back() << "_readonly.h\"\n";

 namespace_open(out, options.get_name_space());

 out << " const char * schema_string = ";
 write_string(out, std::string(schema.data(), schema.size()));
 out << ";\n";
 out << " const size_t schema_string_size = " << schema.size();
 out << ";\n";

 namespace_close(out, options.get_name_space());
}

/////////////////////////////////////////////////////////////////////////////
static void generate_cpp
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options
)
{
 const std::string &file_name = options.get_name_space().back();
 const std::string ns = namespace_string(options.get_name_space());

 out << "#include \"" << file_name << "_readonly.cpp\"\n";
 out << "#include \"" << file_name << ".h\"\n";
 out << "#include \"joedb/Exception.h\"\n";
 out << "#include \"joedb/Writable.h\"\n";
 out << "#include \"joedb/journal/Readonly_Memory_File.h\"\n";
 out << '\n';
 out << "#include <ctime>\n";

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
 void Generic_File_Database::initialize()
 ////////////////////////////////////////////////////////////////////////////
 {
  max_record_id = size_t(journal.get_checkpoint_position());
  ready_to_write = false;
  journal.replay_log(*this);
  ready_to_write = true;
  max_record_id = 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File_Database::auto_upgrade()
 ////////////////////////////////////////////////////////////////////////////
 {
  const size_t file_schema_size = schema_file.get_data().size();

  if (file_schema_size < schema_string_size)
  {
   journal.comment("Automatic schema upgrade");

   joedb::Readonly_Memory_File schema_file(schema_string, schema_string_size);
   joedb::Readonly_Journal schema_journal(schema_file);

   schema_journal.set_position(int64_t(file_schema_size));
   schema_journal.play_until_checkpoint(journal);

   schema_journal.set_position(int64_t(file_schema_size));
   upgrading_schema = true;
   schema_journal.play_until_checkpoint(*this);
   upgrading_schema = false;

   journal.comment("End of automatic schema upgrade");
   journal.default_checkpoint();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Generic_File_Database::Generic_File_Database
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Generic_File &file,
  bool upgrade,
  joedb::Commit_Level commit_level
 ):
  journal(file, commit_level)
 {
  initialize();
  if (upgrade)
  {
   check_schema();
   auto_upgrade();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 Generic_File_Database::Generic_File_Database
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Generic_File &file,
  joedb::Commit_Level commit_level
 ):
  Generic_File_Database(file, true, commit_level)
 {
 }
)RRR";

 namespace_close(out, options.get_name_space());
}

/////////////////////////////////////////////////////////////////////////////
static void generate_interpreted_h
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options
)
{
 const std::vector<std::string> &ns = options.get_name_space();

 namespace_include_guard(out, "Interpreted_File", options.get_name_space());

 out << '\n';
 out << "#include \"" << ns.back() << "_readonly.h\"\n";
 out << '\n';

 namespace_open(out, options.get_name_space());

 out << R"RRR(
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_File: public joedb::Memory_File
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Interpreted_File(std::istream &file);
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Database: public Readonly_Database
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Interpreted_Database(std::istream &file):
    Readonly_Database(Interpreted_File(file))
   {
   }

   Interpreted_Database(std::istream &&file): Interpreted_Database(file) {}

   Interpreted_Database(const char *file_name);
 };

)RRR";

 namespace_close(out, options.get_name_space());

 out << "\n#endif\n";
}

/////////////////////////////////////////////////////////////////////////////
static void generate_interpreted_cpp
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options
)
{
 const std::vector<std::string> &ns = options.get_name_space();

 out << "#include \"" << ns.back() << "_interpreted.h\"\n";
 out << "#include \"joedb/journal/Interpreted_File.h\"\n\n";
 out << "#include <fstream>\n";

 namespace_open(out, ns);

 out << R"RRR(
 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File::Interpreted_File(std::istream &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  write_data(schema_string, schema_string_size);
  set_mode(joedb::Open_Mode::write_existing);
  append_interpreted_commands(*this, file);
 }

 ////////////////////////////////////////////////////////////////////////////
 Interpreted_Database::Interpreted_Database(const char *file_name):
 ////////////////////////////////////////////////////////////////////////////
  Interpreted_Database(std::ifstream(file_name))
 {
 }
)RRR";

 namespace_close(out, ns);
}

/////////////////////////////////////////////////////////////////////////////
static void write_initial_comment
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options,
 const char *exe_path
)
{
 out << "/*///////////////////////////////////////////////////////////////////////////\n";
 out << "//\n";
 out << "// This code was automatically generated by the joedb compiler\n";
 out << "// https://www.remi-coulom.fr/joedb/\n";
 out << "//\n";
 out << "// Path to compiler: " << exe_path << '\n';
 out << "// Version: " << joedb::get_version() << '\n';
 out << "// joedbc compilation time: " << __DATE__ << ' ' << __TIME__ << '\n';
 out << "// Generation of this file: " << get_time_string_of_now() << '\n';
 out << "//\n";
 out << "///////////////////////////////////////////////////////////////////////////*/\n";
}

/////////////////////////////////////////////////////////////////////////////
namespace joedb
/////////////////////////////////////////////////////////////////////////////
{
 class Custom_Collector: public Writable
 {
  private:
   std::vector<std::string> &names;

  public:
   explicit Custom_Collector(std::vector<std::string> &names): names(names)
   {
   }

   void custom(const std::string &name) final
   {
    if (!is_identifier(name))
     throw Exception("custom: invalid identifier");
    names.emplace_back(name);
   }
 };
}

/////////////////////////////////////////////////////////////////////////////
static int joedbc_main(int argc, char **argv)
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

 const char * const exe_path = argv[0];

 //
 // Read file.joedbi
 //
 Database db;
 Memory_File schema_file;
 std::vector<std::string> custom_names;

 {
  std::ifstream joedbi_file(argv[1]);
  if (!joedbi_file.good())
  {
   std::cerr << "Error: could not open " << argv[1] << '\n';
   return 1;
  }

  Writable_Journal journal(schema_file);
  Selective_Writable schema_writable(journal, Selective_Writable::schema);
  Custom_Collector custom_collector(custom_names);

  Multiplexer multiplexer{db, schema_writable, custom_collector};
  Interpreter interpreter(db, multiplexer, nullptr, nullptr, 0);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
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
  std::ofstream h_file
  (
   compiler_options.get_name_space().back() + "_readonly.h",
   std::ios::trunc
  );
  write_initial_comment(h_file, compiler_options, exe_path);
  generate_readonly_h(h_file, compiler_options);
 }
 {
  std::ofstream cpp_file
  (
   compiler_options.get_name_space().back() + "_readonly.cpp",
   std::ios::trunc
  );
  write_initial_comment(cpp_file, compiler_options, exe_path);
  generate_readonly_cpp(cpp_file, compiler_options, schema_file.get_data());
 }
 {
  std::ofstream h_file
  (
   compiler_options.get_name_space().back() + ".h",
   std::ios::trunc
  );
  write_initial_comment(h_file, compiler_options, exe_path);
  generate_h(h_file, compiler_options);
 }
 {
  std::ofstream cpp_file
  (
   compiler_options.get_name_space().back() + ".cpp",
   std::ios::trunc
  );
  write_initial_comment(cpp_file, compiler_options, exe_path);
  generate_cpp(cpp_file, compiler_options);
 }
 {
  std::ofstream h_file
  (
   compiler_options.get_name_space().back() + "_interpreted.h",
   std::ios::trunc
  );
  write_initial_comment(h_file, compiler_options, exe_path);
  generate_interpreted_h(h_file, compiler_options);
 }
 {
  std::ofstream cpp_file
  (
   compiler_options.get_name_space().back() + "_interpreted.cpp",
   std::ios::trunc
  );
  write_initial_comment(cpp_file, compiler_options, exe_path);
  generate_interpreted_cpp(cpp_file, compiler_options);
 }

 if (compiler_options.get_generate_c_wrapper())
 {
  std::ofstream header
  (
   compiler_options.get_name_space().back() + "_wrapper.h",
   std::ios::trunc
  );

  std::ofstream body
  (
   compiler_options.get_name_space().back() + "_wrapper.cpp",
   std::ios::trunc
  );

  write_initial_comment(header, compiler_options, exe_path);
  write_initial_comment(body, compiler_options, exe_path);
  joedb::generate_c_wrapper(header, body, compiler_options);
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedbc_main, argc, argv);
}
