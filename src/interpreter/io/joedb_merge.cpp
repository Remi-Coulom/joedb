#include "File.h"
#include "Journal_File.h"
#include "Stream_File.h"
#include "Database.h"
#include "Selective_Writeable.h"
#include "Multiplexer.h"
#include "dump.h"
#include "merge.h"

#include <iostream>
#include <sstream>
#include <memory>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 int merge(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 3)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " <db_1.joedb> ... <db_N.joedb> <output.joedb>\n";
   return 1;
  }
 
  //
  // Create output file
  //
  File output_file(argv[argc - 1], Open_Mode::create_new);
  Journal_File output_journal(output_file);

  //
  // Storage for all databases
  //
  const size_t input_files = size_t(argc - 2);
  std::vector<std::string> schema(input_files);
  std::vector<std::unique_ptr<Database>> db(input_files);

  //
  // Load all input databases
  //
  for (size_t i = 0; i < input_files; i++)
  {
   db[i].reset(new Database());

   File input_file(argv[i + 1], Open_Mode::read_existing);
   Readonly_Journal input_journal(input_file);

   std::stringstream schema_stream;
   Stream_File schema_file(schema_stream, Open_Mode::create_new);
   Journal_File schema_journal(schema_file);
   Selective_Writeable schema_filter
   (
    schema_journal,
    Selective_Writeable::Mode::schema
   );

   Selective_Writeable output_schema
   (
    output_journal,
    Selective_Writeable::Mode::schema
   );

   Multiplexer multiplexer;
   if (i == 0)
    multiplexer.add_writeable(output_schema);
   multiplexer.add_writeable(schema_filter);
   multiplexer.add_writeable(*db[i]);

   input_journal.replay_log(multiplexer);

   schema_file.flush();
   schema[i] = schema_stream.str();
  }

  //
  // Check that they have all the same schema
  //
  for (size_t i = 1; i < input_files; i++)
   if (schema[i] != schema[0])
    throw Exception
    (
     argv[i + 1] + std::string(" does not have the same schema as ") + argv[1]
    );
 
  //
  // Merge all input files into the output file
  //
  std::unique_ptr<Database> merged_db(db[0].release());
  for (size_t i = 1; i < input_files; i++)
  {
   merge(*merged_db, *db[i]);
   db[i].reset();
  }
  dump_data(*merged_db, output_journal);
 
  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::merge(argc, argv);
}
