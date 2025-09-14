#ifndef joedb_iostream_declared
#define joedb_iostream_declared

#include "joedb/journal/filebuf.h"

#include <iostream>

namespace joedb
{
 /// @ingroup journal
 class iostream: private joedb::filebuf, public std::iostream
 {
  public:
   iostream(joedb::Abstract_File &file):
    filebuf(file),
    std::iostream(static_cast<std::streambuf *>(this))
   {
   }
 };
}

#endif
