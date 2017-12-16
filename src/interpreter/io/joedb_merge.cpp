#include "File.h"
#include "Journal_File.h"
#include "Stream_File.h"
#include "Database.h"
#include "Selective_Writeable.h"

#include <iostream>
#include <sstream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 int merge(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 4)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " <db_1.joedb> ... <db_N.joedb> <output.joedb>\n";
   return 1;
  }
 
  Database db;
  std::string schema;
 
  const int input_files = argc - 2;

  //
  // Check for identical schema
  //
  for (int i = 0; i < input_files; i++)
  {
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

   input_journal.replay_log(schema_filter);

   if (i == 0)
    schema = schema_stream.str();
   else if (schema != schema_stream.str())
    throw Exception
    (
     argv[i + 1] + std::string(" does not have the same schema as ") + argv[1]
    );
  }
 
  //
  // Create output file
  //
  File output_file(argv[argc - 1], Open_Mode::create_new);
  Journal_File output_journal(output_file);

  //
  // Merge all input files into the output file
  //
 
  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::merge(argc, argv);
}
