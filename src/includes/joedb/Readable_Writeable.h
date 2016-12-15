#ifndef joedb_Readable_Writeable_declared
#define joedb_Readable_Writeable_declared

#include "Readable.h"
#include "Writeable.h"

namespace joedb
{
 class Table;

 class Readable_Writeable:
  public virtual Readable,
  public virtual Writeable
 {
 };
}

#endif
