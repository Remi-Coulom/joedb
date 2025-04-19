#include "joedb/ui/process_journal_pair.h"
#include "joedb/journal/File.h"

#include <iostream>
#include <sstream>
#include <optional>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int process_journal_pair
 ////////////////////////////////////////////////////////////////////////////
 (
  int argc,
  char **argv,
  void (*process)(Readonly_Journal &, Writable_Journal &, int64_t checkpoint)
 )
 {
  int64_t checkpoint = 0;
  bool ignore_errors = false;
  int arg_index = 1;

  if (arg_index + 2 < argc && std::string(argv[arg_index]) == "--ignore-errors")
  {
   ignore_errors = true;
   arg_index++;
  }

  if (arg_index + 3 < argc && std::string(argv[arg_index]) == "--checkpoint")
  {
   std::istringstream(argv[arg_index + 1]) >> checkpoint;
   arg_index += 2;
  }

  if (arg_index + 3 != argc)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--ignore-errors] [--checkpoint N] <input.joedb> <output.joedb> <blobs.joedb>\n";
   return 1;
  }

  const char *input_file_name = argv[arg_index];
  const char *output_file_name = argv[arg_index + 1];
  const char *blobs_file_name = argv[arg_index + 2];

  File input_file(input_file_name, Open_Mode::read_existing);

  Readonly_Journal input_journal
  (
   input_file,
   ignore_errors ?
    Readonly_Journal::Check::none :
    Readonly_Journal::Check::all
  );

  File output_file(output_file_name, Open_Mode::create_new);
  Writable_Journal output_journal(output_file);

  std::optional<joedb::File> blobs_file;

  if (*blobs_file_name)
  {
   blobs_file.emplace(blobs_file_name, Open_Mode::read_existing);
   input_file.blob_file = &(*blobs_file);
  }

  process(input_journal, output_journal, checkpoint);

  return 0;
 }
}
