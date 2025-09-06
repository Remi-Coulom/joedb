#ifndef joedb_iostream_declared
#define joedb_iostream_declared

#include "joedb/journal/streambuf.h"

#include <iostream>

namespace joedb
{
 class iostream: private joedb::streambuf, public std::iostream
 {
  public:
   iostream(joedb::Abstract_File &file):
    streambuf(file),
    std::iostream(static_cast<std::streambuf *>(this))
   {
   }
 };
}

#endif
