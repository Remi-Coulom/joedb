#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/ui/dump.h"
#include "joedb/ui/merge.h"
#include "joedb/ui/main_exception_catcher.h"

#include <iostream>
#include <memory>
#include <iomanip>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int merge_main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 2)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " <db_1.joedb> ... <db_N.joedb> <output.joedb>\n";
   std::cerr << "or read file names from input stream: " << argv[0];
   std::cerr << " <output.joedb> <file_list.txt\n";
   std::cerr << "Note: output file must not already exist\n";
   return 1;
  }

  //
  // Create output file
  //
  File output_file(argv[argc - 1], Open_Mode::create_new);
  Writable_Journal output_journal(output_file);

  //
  // List of files to be merged
  //
  std::vector<std::string> file_names;

  for (int i = 1; i < argc - 1; i++)
   file_names.emplace_back(argv[i]);

  if (file_names.empty())
  {
   std::cerr << "No input file on the command line: reading file names from standard input.\n";

   std::string file_name;
   while (std::cin >> file_name) // note: no file name with white space
    file_names.emplace_back(std::move(file_name));
  }

  if (file_names.empty())
  {
   std::cerr << "Error: no input file\n";
   return 1;
  }

  //
  // Build merged db by looping over all files
  //
  std::string reference_schema;
  std::unique_ptr<Database> merged_db;
  const int width = int(std::to_string(file_names.size()).size());
  int errors = 0;

  for (size_t i = 0; i < file_names.size(); i++)
  {
   std::cerr << std::setw(width) << i + 1 << " / ";
   std::cerr << file_names.size() << ": " << file_names[i] << "...";

   try
   {
    std::unique_ptr<Database> db(new Database());

    File input_file(file_names[i], Open_Mode::read_existing);
    Readonly_Journal input_journal(input_file);

    Memory_File schema_file;
    Writable_Journal schema_journal(schema_file);

    {
     Selective_Writable schema_filter
     (
      schema_journal,
      Selective_Writable::Mode::schema
     );

     std::unique_ptr<Selective_Writable> output_schema;
     std::unique_ptr<Multiplexer> multiplexer;

     if (merged_db)
     {
      multiplexer.reset(new Multiplexer{*db, schema_filter});
     }
     else
     {
      output_schema.reset
      (
       new Selective_Writable
       (
        output_journal,
        Selective_Writable::Mode::schema
       )
      );

      multiplexer.reset(new Multiplexer{*db, schema_filter, *output_schema});
     }

     input_journal.raw_play_until_checkpoint(*multiplexer);
    }

    //
    // Check that all databases have the same schema
    //
    schema_journal.soft_checkpoint();
    if (!merged_db)
     reference_schema = schema_file.get_data();
    else if (schema_file.get_data() != reference_schema)
     throw Exception
     (
      file_names[i] +
      std::string(" does not have the same schema as ") +
      file_names[0]
     );

    //
    // Merge into the in-memory database
    //
    if (!merged_db)
     merged_db = std::move(db);
    else
     merge(*merged_db, *db);
   }
   catch (const Exception &e)
   {
    std::cerr << ' ' << e.what();
    errors++;
   }

   std::cerr << '\n';
  }

  if (merged_db)
  {
   dump_data(*merged_db, output_journal);

   if (errors > 0)
    std::cerr << "Number of errors: " << errors << '\n';

   return 0;
  }
  else
   return 1;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::merge_main, argc, argv);
}
