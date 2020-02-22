#ifndef joedb_Readable_Writable_declared
#define joedb_Readable_Writable_declared

#include "Readable.h"
#include "Writable.h"

namespace joedb
{
 class Table;

 class Readable_Writable:
  public virtual Readable,
  public virtual Writable
 {
 };
}

#endif
