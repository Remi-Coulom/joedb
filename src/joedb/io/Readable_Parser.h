#ifndef joedb_Readable_Parser_declared
#define joedb_Readable_Parser_declared

#include "joedb/Type.h"

#include <iosfwd>

namespace joedb
{
 class Readable;

 ////////////////////////////////////////////////////////////////////////////
 class Readable_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   const Readable &readable;

   Type parse_type(std::istream &in, std::ostream &out) const;
   Table_Id parse_table(std::istream &in, std::ostream &out) const;

  public:
   Readable_Parser(const Readable &readable): readable(readable)
   {
   }
 };
}

#endif
