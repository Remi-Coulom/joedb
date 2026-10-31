#ifndef joedb_joedb_convert_declared
#define joedb_joedb_convert_declared

#include "joedb/ui/process_journal_pair.h"

#include <istream>
#include <ostream>

namespace joedb
{
 inline void convert_process
 (
  Readonly_Journal &input,
  Writable_Journal &output,
  int64_t checkpoint
 )
 {
  if (checkpoint > input.get_checkpoint() || checkpoint == 0)
   checkpoint = input.get_checkpoint();
  input.play_until(output, checkpoint);
  output.soft_checkpoint();
 }

 inline int convert
 (
  Arguments &arguments,
  std::istream &in,
  std::ostream &out,
  std::ostream &err
 )
 {
  return process_journal_pair(arguments, err, convert_process);
 }
}

#endif
