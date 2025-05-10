#ifndef joedb_process_journal_pair_declared
#define joedb_process_journal_pair_declared

#include "joedb/journal/Writable_Journal.h"
#include "joedb/ui/Arguments.h"

namespace joedb
{
 /// @ingroup ui
 int process_journal_pair
 (
  Arguments &arguments,
  void (*process)(Readonly_Journal &, Writable_Journal &, int64_t checkpoint)
 );
}

#endif
