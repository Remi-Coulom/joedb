#include "File.h"
#include "Journal_File.h"
#include "Stream_File.h"
#include "Database.h"
#include "Selective_Writable.h"
#include "Multiplexer.h"
#include "dump.h"
#include "merge.h"
#include "main_exception_catcher.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <iomanip>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 int merge_main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 2)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " <db_1.joedb> ... <db_N.joedb> <output.joedb>\n";
   std::cerr << "or read file names from input stream: " << argv[0];
   std::cerr << " <output.joedb>\n";
   return 1;
  }
 
  //
  // Create output file
  //
  File output_file(argv[argc - 1], Open_Mode::create_new);
  Journal_File output_journal(output_file);

  //
  // List of files to be merged
  //
  std::vector<std::string> file_names;

  for (int i = 1; i < argc - 1; i++)
   file_names.push_back(argv[i]);

  if (file_names.size() == 0)
  {
   std::cerr << "No input file on the command line: reading file names from standard input.\n";

   std::string file_name;
   while (std::cin >> file_name)
    file_names.push_back(file_name);
  }

  if (file_names.size() == 0)
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
    std::unique_ptr<joedb::Database> db(new Database());

    File input_file(file_names[i], Open_Mode::read_existing);
    Readonly_Journal input_journal(input_file);

    std::stringstream schema_stream;
    Stream_File schema_file(schema_stream, Open_Mode::create_new);
    Journal_File schema_journal(schema_file);
    Selective_Writable schema_filter
    (
     schema_journal,
     Selective_Writable::Mode::schema
    );

    Selective_Writable output_schema
    (
     output_journal,
     Selective_Writable::Mode::schema
    );

    Multiplexer multiplexer;
    if (!merged_db)
     multiplexer.add_writable(output_schema);
    multiplexer.add_writable(schema_filter);
    multiplexer.add_writable(*db);

    input_journal.replay_log(multiplexer);

    //
    // Check that all databases have the same schema
    //
    schema_file.flush();
    std::string schema = schema_stream.str();
    if (!merged_db)
     reference_schema = schema;
    else if (schema != reference_schema)
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
     merged_db.reset(db.release());
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
