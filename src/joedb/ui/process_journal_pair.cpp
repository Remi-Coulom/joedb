#include "joedb/ui/process_journal_pair.h"
#include "joedb/journal/File.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int process_journal_pair
 ////////////////////////////////////////////////////////////////////////////
 (
  Arguments &arguments,
  void (*process)(Readonly_Journal &, Writable_Journal &, int64_t checkpoint)
 )
 {
  const bool ignore_errors = arguments.has_option("ignore_errors");
  const int64_t until = arguments.get_option<int64_t>
  (
   "until",
   "checkpoint",
   0
  );
  const std::string_view input = arguments.get_next("input.joedb");
  const std::string_view output = arguments.get_next("output.joedb");

  if (arguments.has_missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  File input_file(input.data(), Open_Mode::read_existing);

  Readonly_Journal input_journal
  (
   Journal_Construction_Lock(input_file, ignore_errors)
  );

  File output_file(output.data(), Open_Mode::create_new);
  Writable_Journal output_journal(output_file);

  process(input_journal, output_journal, until);

  return 0;
 }
}
