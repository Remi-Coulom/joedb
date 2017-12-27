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
#include <iomanip>

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

  const size_t input_files = size_t(argc - 2);
  std::string reference_schema;
  std::unique_ptr<Database> merged_db;

  //
  // Loop over all file names
  //
  for (size_t i = 0; i < input_files; i++)
  {
   std::cerr << std::setw(5) << i << ' ' << argv[i + 1] << "...";

   std::unique_ptr<joedb::Database> db(new Database());

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
   multiplexer.add_writeable(*db);

   input_journal.replay_log(multiplexer);

   //
   // Check that all databases have the same schema
   //
   schema_file.flush();
   std::string schema = schema_stream.str();
   if (i == 0)
    reference_schema = schema;
   else if (schema != reference_schema)
    throw Exception
    (
     argv[i + 1] + std::string(" does not have the same schema as ") + argv[1]
    );

   //
   // Merge into the in-memory database
   //
   if (i == 0)
    merged_db.reset(db.release());
   else
    merge(*merged_db, *db);

   std::cerr << '\n';
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
