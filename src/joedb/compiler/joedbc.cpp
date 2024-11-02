#include "joedb/journal/Writable_Journal.h"
#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/get_time_string.h"
#include "joedb/io/type_io.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/compiler/Compiler_Options.h"
#include "joedb/compiler/Compiler_Options_io.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/is_identifier.h"
#include "joedb/get_version.h"
#include "joedb/interpreter/Database.h"

#include "joedb/compiler/generator/Database_h.h"
#include "joedb/compiler/generator/Database_cpp.h"
#include "joedb/compiler/generator/readonly_cpp.h"
#include "joedb/compiler/generator/Generic_File_Database_h.h"

#include <fstream>
#include <filesystem>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
static bool has_values(const Database &db)
/////////////////////////////////////////////////////////////////////////////
{
 for (const auto &[tid, tname]: db.get_tables())
 {
  if (db.get_freedom(tid).size() > 0)
   return true;
 }
 return false;
}

/////////////////////////////////////////////////////////////////////////////
static void generate_h(std::ostream &out, const Compiler_Options &options)
/////////////////////////////////////////////////////////////////////////////
{
 const Database &db = options.get_db();
 auto tables = db.get_tables();

 namespace_include_guard(out, "writable", options.get_name_space());

 out << R"RRR(
#include "readonly.h"
#include "Generic_File_Database.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Connection.h"
)RRR";

 namespace_open(out, options.get_name_space());

 //
 // Database
 //
 out << R"RRR(
 class Client_Data;
 class Client;
)RRR";

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
    joedb::Readonly_Journal::Check check = joedb::Readonly_Journal::Check::all,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    File_Database_Parent(file_name, mode),
    Generic_File_Database(file, check, commit_level)
   {
   }

   File_Database
   (
    const std::string &file_name,
    joedb::Open_Mode mode = joedb::Open_Mode::write_existing_or_create_new,
    joedb::Readonly_Journal::Check check = joedb::Readonly_Journal::Check::all,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    File_Database(file_name.c_str(), mode, check, commit_level)
   {
   }
 };

)RRR";

 for (const auto &[tid, tname]: tables)
 {
  const bool single_row = options.get_table_options(tid).single_row;

  if (!single_row)
  {
   out << " inline void Generic_File_Database::clear_" << tname << "_table()\n";
   out << " {\n";
   out << "  while (!get_" << tname << "_table().is_empty())\n";
   out << "   delete_" << tname << "(get_" << tname << "_table().last());\n";
   out << " }\n";
   out << '\n';
  }
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
    joedb::Readonly_Journal::Check check,
    joedb::Commit_Level commit_level
   ):
    db(file, false, check, commit_level)
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
 };

 ////////////////////////////////////////////////////////////////////////////
 class Client:
 ////////////////////////////////////////////////////////////////////////////
  private Client_Data,
  public joedb::Client,
  public joedb::Blob_Reader
 {
  private:
   int64_t schema_checkpoint;

   void play_journal_and_throw_if_schema_changed()
   {
    db.play_journal();
    if (db.schema_journal.get_checkpoint_position() > schema_checkpoint)
     throw joedb::Exception("Can't upgrade schema during pull");
    db.check_single_row();
   }

  public:
   Client
   (
    joedb::Generic_File &file,
    joedb::Connection &connection,
    bool content_check = true,
    joedb::Readonly_Journal::Check check = joedb::Readonly_Journal::Check::all,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    Client_Data(file, check, commit_level),
    joedb::Client(*static_cast<Client_Data *>(this), connection, content_check)
   {
    if (get_checkpoint_difference() > 0)
     push_unlock();

    db.play_journal(); // makes transaction shorter if db is big
    joedb::Client::transaction([this](joedb::Client_Data &data){
     db.initialize();
    });

    schema_checkpoint = db.schema_journal.get_checkpoint_position();
   }

   const Database &get_database() const
   {
    return db;
   }

   std::string read_blob_data(joedb::Blob blob) final
   {
    return db.read_blob_data(blob);
   }

   int64_t pull()
   {
    const int64_t result = joedb::Client::pull();
    play_journal_and_throw_if_schema_changed();
    return result;
   }

   template<typename F> void transaction(F transaction)
   {
    joedb::Client::transaction([&](joedb::Client_Data &data)
    {
     play_journal_and_throw_if_schema_changed();
     transaction(db);
    });
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Local_Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   joedb::File file;
   joedb::Connection connection;

   Local_Client_Data(const char *file_name):
    file(file_name, joedb::File::lockable ? joedb::Open_Mode::shared_write : joedb::Open_Mode::write_existing_or_create_new)
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
    Client(Local_Client_Data::file, Local_Client_Data::connection)
   {
   }

   Local_Client(const std::string &file_name):
    Local_Client(file_name.c_str())
   {
   }
 };
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
 const Database_Schema &db = options.get_db();
 auto tables = db.get_tables();

 namespace_include_guard(out, "readonly", options.get_name_space());

 out << R"RRR(
#include "Database.h"
#include "joedb/journal/File.h"
#include "joedb/concurrency/Client.h"

)RRR";

 namespace_open(out, options.get_name_space());

 //
 // Readonly_Database and Client
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

 using Generic_Readonly_Database = Readonly_Database;

 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Client_Data:
 ////////////////////////////////////////////////////////////////////////////
  public joedb::Client_Data,
  public joedb::Blob_Reader
 {
  protected:
   joedb::Readonly_Journal journal;
   Database db;

   Readonly_Client_Data(joedb::File &file):
    journal(file)
   {
    db.initialize_with_readonly_journal(journal);
   }

   bool is_readonly() const override
   {
    return true;
   }

   joedb::Readonly_Journal &get_readonly_journal() override
   {
    return journal;
   }

   std::string read_blob_data(joedb::Blob blob) final
   {
    return journal.read_blob_data(blob);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Pullonly_Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   joedb::Pullonly_Connection connection;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Client:
 ////////////////////////////////////////////////////////////////////////////
  private Readonly_Client_Data,
  private Pullonly_Connection,
  private joedb::Pullonly_Client
 {
  private:
   const int64_t schema_checkpoint;

  public:
   Readonly_Client(joedb::File &file):
    Readonly_Client_Data(file),
    joedb::Pullonly_Client
    (
     *static_cast<Readonly_Client_Data *>(this),
     Pullonly_Connection::connection,
     false
    ),
    schema_checkpoint(db.get_schema_checkpoint())
   {
   }

   const Database &get_database() const {return db;}

   bool pull()
   {
    joedb::Pullonly_Client::pull();
    if (journal.get_position() < journal.get_checkpoint_position())
    {
     journal.play_until_checkpoint(db);
     if (db.get_schema_checkpoint() > schema_checkpoint)
      throw joedb::Exception("Pulled a schema change");
     return true;
    }
    else
     return false;
   }
 };

)RRR";

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

 for (const auto &[tid, tname]: tables)
 {
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
static void generate_cpp
/////////////////////////////////////////////////////////////////////////////
(
 std::ostream &out,
 const Compiler_Options &options
)
{
 const auto &db = options.get_db();
 const auto &tables = db.get_tables();

 out << "#include \"readonly.cpp\"\n";
 out << "#include \"writable.h\"\n";
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

 if (has_values(options.get_db()))
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

     joedb::write_value(out, db, nullptr, tid, record_id, fid);

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

 namespace_close(out, options.get_name_space());
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
 out << "/////////////////////////////////////////////////////////////////////////////\n";
 out << "//\n";
 out << "// This code was automatically generated by the joedb compiler\n";
 out << "// https://www.remi-coulom.fr/joedb/\n";
 out << "//\n";
 out << "// Path to compiler: " << exe_path << '\n';
 out << "// Version: " << joedb::get_version() << '\n';
 out << "// joedbc compilation time: " << __DATE__ << ' ' << __TIME__ << '\n';
 out << "// Generation of this file: " << get_time_string_of_now() << '\n';
 out << "//\n";
 out << "/////////////////////////////////////////////////////////////////////////////\n";
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
 const char * const joedbi_file_name = argv[1];
 const char * const joedbc_file_name = argv[2];

 Compiler_Options options;
 options.exe_path = std::string(exe_path);

 //
 // Read file.joedbi
 //
 {
  std::ifstream joedbi_file(joedbi_file_name);
  if (!joedbi_file)
  {
   std::cerr << "Error: could not open " << joedbi_file_name << '\n';
   return 1;
  }

  Writable_Journal journal(options.schema_file);
  Selective_Writable schema_writable(journal, Selective_Writable::schema);
  Custom_Collector custom_collector(options.custom_names);

  Multiplexer multiplexer{options.db, schema_writable, custom_collector};
  Interpreter interpreter(options.db, multiplexer, nullptr, nullptr, 0);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
  interpreter.main_loop(joedbi_file, std::cerr);
  multiplexer.default_checkpoint();
 }

 for (const auto &[tid, tname]: options.db.get_tables())
  options.table_options[tid];

 //
 // Read file.joedbc
 //
 std::ifstream joedbc_file(joedbc_file_name);
 if (!joedbc_file)
 {
  std::cerr << "Error: could not open " << joedbc_file_name << '\n';
  return 1;
 }


 try
 {
  parse_compiler_options(joedbc_file, options);
 }
 catch(...)
 {
  std::cerr << "Error parsing .joedbc file: " << argv[2] << '\n';
  throw;
 }

 //
 // Generate code
 //
 const std::string dir_name = options.get_name_space().back();
 std::filesystem::create_directory(dir_name);

 {
  std::ofstream h_file
  (
   options.get_name_space().back() + "/readonly.h",
   std::ios::trunc
  );
  write_initial_comment(h_file, options, exe_path);
  generate_readonly_h(h_file, options);
 }
 {
  std::ofstream h_file
  (
   options.get_name_space().back() + "/writable.h",
   std::ios::trunc
  );
  write_initial_comment(h_file, options, exe_path);
  generate_h(h_file, options);
 }
 {
  std::ofstream cpp_file
  (
   options.get_name_space().back() + "/writable.cpp",
   std::ios::trunc
  );
  write_initial_comment(cpp_file, options, exe_path);
  generate_cpp(cpp_file, options);
 }

 generator::Database_h(options).generate();
 generator::Database_cpp(options).generate();
 generator::readonly_cpp(options).generate();

 generator::Generic_File_Database_h(options).generate();

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedbc_main, argc, argv);
}
