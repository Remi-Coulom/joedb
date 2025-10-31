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

 class null_iostream: private joedb::Abstract_File, public joedb::iostream
 {
  public:
   null_iostream():
    Abstract_File(Open_Mode::create_new),
    iostream(*static_cast<joedb::Abstract_File *>(this))
   {
   }
 };
}

#endif
