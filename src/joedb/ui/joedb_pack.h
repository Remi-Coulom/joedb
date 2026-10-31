#ifndef joedb_joedb_pack_declared
#define joedb_joedb_pack_declared

#include "joedb/ui/dump.h"
#include "joedb/ui/process_journal_pair.h"

#include <istream>
#include <ostream>

namespace joedb
{
 inline void pack_process
 (
  Readonly_Journal &input,
  Writable_Journal &output,
  int64_t checkpoint
 )
 {
  pack(input, output);
 }

 inline int pack
 (
  Arguments &arguments,
  std::istream &in,
  std::ostream &out,
  std::ostream &err
 )
 {
  return process_journal_pair(arguments, err, pack_process);
 }
}

#endif
