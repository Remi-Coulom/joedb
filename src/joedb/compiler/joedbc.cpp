#include "joedb/journal/Writable_Journal.h"
#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/compiler/Compiler_Options.h"
#include "joedb/compiler/Compiler_Options_io.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/is_identifier.h"
#include "joedb/interpreter/Database.h"

#include "joedb/compiler/generator/Database_h.h"
#include "joedb/compiler/generator/Database_cpp.h"
#include "joedb/compiler/generator/Readonly_Database_h.h"
#include "joedb/compiler/generator/readonly_cpp.h"

#include "joedb/compiler/generator/Generic_File_Database_h.h"
#include "joedb/compiler/generator/Generic_File_Database_cpp.h"
#include "joedb/compiler/generator/File_Database_h.h"
#include "joedb/compiler/generator/writable_cpp.h"

#include "joedb/compiler/generator/Client_h.h"

#include <fstream>
#include <filesystem>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
static void generate_h(std::ostream &out, const Compiler_Options &options)
/////////////////////////////////////////////////////////////////////////////
{
 const Database &db = options.get_db();
 auto tables = db.get_tables();

 namespace_include_guard(out, "writable", options.get_name_space());

 out << R"RRR(
#include "readonly.h"
#include "File_Database.h"
#include "Client.h"

)RRR";

 namespace_open(out, options.get_name_space());

 //
 // Concurrency
 //
 out << R"RRR(
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
#include "Readonly_Database.h"
#include "joedb/concurrency/Client.h"

)RRR";

 namespace_open(out, options.get_name_space());

 out << R"RRR(
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
  generate_readonly_h(h_file, options);
 }
 {
  std::ofstream h_file
  (
   options.get_name_space().back() + "/writable.h",
   std::ios::trunc
  );
  generate_h(h_file, options);
 }

 generator::Database_h(options).generate();
 generator::Database_cpp(options).generate();
 generator::Readonly_Database_h(options).generate();
 generator::readonly_cpp(options).generate();

 generator::Generic_File_Database_h(options).generate();
 generator::Generic_File_Database_cpp(options).generate();
 generator::File_Database_h(options).generate();
 generator::writable_cpp(options).generate();

 generator::Client_h(options).generate();

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedbc_main, argc, argv);
}
