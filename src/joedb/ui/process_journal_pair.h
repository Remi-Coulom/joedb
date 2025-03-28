#ifndef joedb_process_journal_pair_declared
#define joedb_process_journal_pair_declared

#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 /// \ingroup ui
 int process_journal_pair
 (
  int argc,
  char **argv,
  void (*process)(Readonly_Journal &, Writable_Journal &, int64_t checkpoint)
 );
}

#endif
