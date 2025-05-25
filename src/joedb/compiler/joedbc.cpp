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
 static int joedbc(Arguments &arguments)
 ////////////////////////////////////////////////////////////////////////////
 {
  const std::string_view joedbi_file_name = arguments.get_next("file.joedbi");
  const std::string_view joedbc_file_name = arguments.get_next("file.joedbc");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  Compiler_Options options;
  options.exe_path = arguments[0];

  //
  // Read file.joedbi
  //
  {
   std::ifstream joedbi_file(joedbi_file_name.data());
   if (!joedbi_file)
   {
    std::cerr << "Error: could not open " << joedbi_file_name << '\n';
    return 1;
   }

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
   std::cerr << "Error parsing .joedbc file: " << joedbc_file_name << '\n';
   throw;
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

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::joedbc, argc, argv);
}
