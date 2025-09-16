#include "joedb/is_identifier.h"
#include "joedb/Multiplexer.h"
#include "joedb/Selective_Writable.h"
#include "joedb/compiler/Compiler_Options_io.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File.h"
#include "joedb/journal/fstream.h"
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
#include "joedb/compiler/generator/print_table_h.h"

#include "joedb/compiler/generator/Procedure_h.h"
#include "joedb/compiler/generator/Procedures_h.h"
#include "joedb/compiler/generator/Signatures_h.h"
#include "joedb/compiler/generator/RPC_Client_h.h"

#include <iostream>
#include <filesystem>
#include <regex>

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
  Compiler_Options &options,
  Compiler_Options *parent_options
 )
 {
  const std::string joedbi_file_name = options.base_name + ".joedbi";
  const std::string joedbc_file_name = options.base_name + ".joedbc";

  //
  // Read file.joedbi (write_lock to block concurrent invocations)
  //
  joedb::ifstream joedbi_file(joedbi_file_name, Open_Mode::write_lock);

  {
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
  {
   joedb::ifstream joedbc_file(joedbc_file_name, Open_Mode::read_existing);
   parse_compiler_options(joedbc_file, options);
  }

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

  generator::ids_h(options, parent_options).generate();
  generator::print_table_h(options).generate();

  if (parent_options)
   generator::Procedure_h(options, *parent_options).generate();
  else
   generator::Procedure_h(options, options).generate();

  for (const auto &table: options.db.get_tables())
   generator::introspection_h(options, table).generate();

  //
  // .gitignore
  //
  {
   joedb::File file
   (
    options.output_path + "/" + options.get_name_space_back() + "/.gitignore",
    Open_Mode::write_lock
   );
   file.pwrite("*\n", 2, 0);
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
   std::cerr << " <base_name>.rpc (optional) directory for RPC\n";
   return 1;
  }

  Compiler_Options options;
  options.exe_path = arguments[0];
  options.output_path = ".";
  options.base_name = std::string(base_name);

  compile(options, nullptr);

  //
  // Generate code for procedure message schemas
  //
  {
   std::error_code error_code;

   std::filesystem::directory_iterator iterator
   (
    std::string(base_name) + ".rpc",
    error_code
   );

   if (!error_code)
   {
    for (const auto &dir_entry: iterator)
    {
     if (dir_entry.path().extension() == ".joedbi")
     {
      auto path = dir_entry.path();
      const std::string procedure_name = path.replace_extension("").string();

      Compiler_Options procedure_options;
      procedure_options.exe_path = arguments[0];
      procedure_options.output_path = std::string(base_name) + "/rpc";
      procedure_options.base_name = procedure_name;

      compile(procedure_options, &options);
     }
    }
   }
  }

  //
  // Find list of procedures in Service.h
  //
  joedb::Memory_File memory_file;

  try
  {
   joedb::File file
   (
    options.base_name + ".rpc/Service.h",
    joedb::Open_Mode::read_existing
   );
   file.copy_to(memory_file);
  }
  catch (...)
  {
  }

  if (memory_file.get_size())
  {
   std::filesystem::copy
   (
    options.base_name + ".rpc/Service.h",
    options.base_name + "/rpc/Service.h",
    std::filesystem::copy_options::overwrite_existing
   );

   const std::string &s = memory_file.get_data();

   std::vector<generator::Procedure> procedures;

   {
    const std::regex pattern("void\\s+(\\w+)\\s*\\(\\s*(\\w+)\\s*::\\s*Writable_Database\\s*&\\s*\\w+\\s*\\)");

    for
    (
     std::sregex_iterator i{s.begin(), s.end(), pattern};
     i != std::sregex_iterator();
     ++i
    )
    {
     procedures.push_back({(*i)[1], (*i)[2]});
    }
   }

   if (!procedures.empty())
   {
    generator::Procedures_h(options, procedures).generate();
    generator::Signatures_h(options, procedures).generate();
    generator::RPC_Client_h(options, procedures).generate();
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
