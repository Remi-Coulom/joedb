#include "joedb/is_identifier.h"
#include "joedb/Multiplexer.h"
#include "joedb/Selective_Writable.h"
#include "joedb/compiler/Compiler_Options_io.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/ui/Interpreter.h"
#include "joedb/ui/main_wrapper.h"

#include "joedb/compiler/generator/Database_h.h"
#include "joedb/compiler/generator/Database_Writable_h.h"
#include "joedb/compiler/generator/Database_Writable_cpp.h"
#include "joedb/compiler/generator/Readonly_Database_h.h"
#include "joedb/compiler/generator/Types_h.h"
#include "joedb/compiler/generator/Readable_h.h"
#include "joedb/compiler/generator/readonly_h.h"
#include "joedb/compiler/generator/readonly_cpp.h"

#include "joedb/compiler/generator/Writable_Database_h.h"
#include "joedb/compiler/generator/Writable_Database_cpp.h"
#include "joedb/compiler/generator/File_Database_h.h"
#include "joedb/compiler/generator/Memory_Database_h.h"
#include "joedb/compiler/generator/Interpreted_File_Database_h.h"
#include "joedb/compiler/generator/Multiplexer_h.h"
#include "joedb/compiler/generator/Readonly_Interpreted_File_Database_h.h"
#include "joedb/compiler/generator/writable_h.h"
#include "joedb/compiler/generator/writable_cpp.h"

#include "joedb/compiler/generator/Client_h.h"
#include "joedb/compiler/generator/File_Client_h.h"
#include "joedb/compiler/generator/Readonly_Client_h.h"

#include "joedb/compiler/generator/ids_h.h"
#include "joedb/compiler/generator/introspection_h.h"

#include <iostream>
#include <filesystem>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Custom_Collector: public Dummy_Writable
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::string> &names;

  public:
   explicit Custom_Collector(std::vector<std::string> &names): names(names)
   {
   }

   void custom(const std::string &name) override
   {
    if (!is_identifier(name))
     throw Exception("custom: invalid identifier");
    names.emplace_back(name);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 static void compile
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &exe_path,
  const std::string &output_path,
  const std::string &base_name
 )
 {
  Compiler_Options options;
  options.exe_path = exe_path;
  options.output_path = output_path;

  const std::string joedbi_file_name = std::string(base_name) + ".joedbi";
  const std::string joedbc_file_name = std::string(base_name) + ".joedbc";

  //
  // Read file.joedbi
  //
  {
   std::ifstream joedbi_file(joedbi_file_name.data());
   if (!joedbi_file)
    throw Exception("could not open " + joedbi_file_name);

   Writable_Journal journal(options.schema_file);
   Selective_Writable schema_writable(journal, Selective_Writable::schema);
   Custom_Collector custom_collector(options.custom_names);

   Multiplexer multiplexer{options.db, schema_writable, custom_collector};
   Interpreter interpreter(options.db, multiplexer, Record_Id::null);
   interpreter.set_echo(false);
   interpreter.set_rethrow(true);
   interpreter.main_loop(joedbi_file, std::cerr);
   multiplexer.soft_checkpoint();
  }

  for (const auto &[tid, tname]: options.db.get_tables())
   options.table_options[tid];

  //
  // Read file.joedbc
  //
  std::ifstream joedbc_file(joedbc_file_name.data());
  if (!joedbc_file)
   throw Exception("Error: could not open " + joedbc_file_name);

  parse_compiler_options(joedbc_file, options);

  //
  // Generate code
  //
  generator::Database_h(options).generate();
  generator::Database_Writable_h(options).generate();
  generator::Database_Writable_cpp(options).generate();
  generator::Readonly_Database_h(options).generate();
  generator::Types_h(options).generate();
  generator::Readable_h(options).generate();

  generator::readonly_h(options).generate();
  generator::readonly_cpp(options).generate();

  generator::Writable_Database_h(options).generate();
  generator::Writable_Database_cpp(options).generate();
  generator::File_Database_h(options).generate();
  generator::Memory_Database_h(options).generate();
  generator::Readonly_Interpreted_File_Database_h(options).generate();
  generator::Interpreted_File_Database_h(options).generate();
  generator::Multiplexer_h(options).generate();

  generator::writable_h(options).generate();
  generator::writable_cpp(options).generate();

  generator::Client_h(options).generate();
  generator::File_Client_h(options).generate();
  generator::Readonly_Client_h(options).generate();

  generator::ids_h(options).generate();

  for (const auto &table: options.db.get_tables())
   generator::introspection_h(options, table).generate();

  //
  // .gitignore
  //
  {
   std::ofstream ofs(output_path + "/" + options.get_name_space_back() + "/.gitignore", std::ios::trunc);
   ofs << "*\n";
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 static int joedbc(Arguments &arguments)
 ////////////////////////////////////////////////////////////////////////////
 {
  const std::string_view base_name = arguments.get_next("<base_name>");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   std::cerr << "joedbc will read:\n";
   std::cerr << " <base_name>.joedbi for the schema definition\n";
   std::cerr << " <base_name>.joedbc for compiler options\n";
   std::cerr << " <base_name>.procedures (optional) directory of procedures\n";
   return 1;
  }

  const std::string exe_path(arguments[0]);
  compile(exe_path, ".", std::string(base_name));

  {
   std::error_code ec;

   std::filesystem::directory_iterator iterator
   (
    std::string(base_name) + ".procedures",
    ec
   );

   for (const auto &dir_entry: iterator)
   {
    if (dir_entry.path().extension() == ".joedbi")
    {
     auto path = dir_entry.path();
     const std::string procedure_name = path.replace_extension("");
     const std::string output_path = std::string(base_name) + "/procedures";
     compile(exe_path, output_path, procedure_name);
    }
   }
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::joedbc, argc, argv);
}
