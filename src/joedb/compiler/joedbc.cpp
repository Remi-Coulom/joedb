#include "joedb/is_identifier.h"
#include "joedb/Multiplexer.h"
#include "joedb/Selective_Writable.h"
#include "joedb/compiler/Compiler_Options_io.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"

#include "joedb/compiler/generator/Database_h.h"
#include "joedb/compiler/generator/Database_cpp.h"
#include "joedb/compiler/generator/Readonly_Database_h.h"
#include "joedb/compiler/generator/Types_h.h"
#include "joedb/compiler/generator/Readable_h.h"
#include "joedb/compiler/generator/readonly_h.h"
#include "joedb/compiler/generator/readonly_cpp.h"

#include "joedb/compiler/generator/Buffered_File_Database_h.h"
#include "joedb/compiler/generator/Buffered_File_Database_cpp.h"
#include "joedb/compiler/generator/File_Database_h.h"
#include "joedb/compiler/generator/Interpreted_Database_h.h"
#include "joedb/compiler/generator/Readonly_Interpreted_Database_h.h"
#include "joedb/compiler/generator/writable_h.h"
#include "joedb/compiler/generator/writable_cpp.h"

#include "joedb/compiler/generator/Client_h.h"
#include "joedb/compiler/generator/Local_Client_h.h"
#include "joedb/compiler/generator/Readonly_Client_h.h"

#include "joedb/compiler/generator/ids_h.h"
#include "joedb/compiler/generator/introspection_h.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Custom_Collector: public Writable
 ////////////////////////////////////////////////////////////////////////////
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

 ////////////////////////////////////////////////////////////////////////////
 static int joedbc_main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
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
   Interpreter interpreter(options.db, multiplexer, nullptr, multiplexer, 0);
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
  generator::Database_h(options).generate();
  generator::Database_cpp(options).generate();
  generator::Readonly_Database_h(options).generate();
  generator::Types_h(options).generate();
  generator::Readable_h(options).generate();
  generator::readonly_h(options).generate();
  generator::readonly_cpp(options).generate();

  generator::Buffered_File_Database_h(options).generate();
  generator::Buffered_File_Database_cpp(options).generate();
  generator::File_Database_h(options).generate();
  generator::Readonly_Interpreted_Database_h(options).generate();
  generator::Interpreted_Database_h(options).generate();
  generator::writable_h(options).generate();
  generator::writable_cpp(options).generate();

  generator::Client_h(options).generate();
  generator::Local_Client_h(options).generate();
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
 return joedb::main_exception_catcher(joedb::joedbc_main, argc, argv);
}
