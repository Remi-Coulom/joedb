#ifndef joedb_streambuf_declared
#define joedb_streambuf_declared

#include <streambuf>

#include "joedb/journal/Buffered_File.h"

namespace joedb
{
 class streambuf: public std::streambuf
 {
  private:
   Buffered_File &file;

  protected:
   virtual int sync() override
   {
    file.flush();
    return 0;
   }

  public:
   streambuf(Buffered_File &file): file(file)
   {
   }
 };
}

#endif
