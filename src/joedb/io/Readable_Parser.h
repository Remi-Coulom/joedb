#ifndef joedb_Readable_Parser_declared
#define joedb_Readable_Parser_declared

#include "joedb/Type.h"

#include <iosfwd>

namespace joedb
{
 class Readable;
 class Blob_Reader;

 ////////////////////////////////////////////////////////////////////////////
 class Readable_Parser
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   const Readable &readable;
   Blob_Reader * const blob_reader;

   Type parse_type(std::istream &in, std::ostream &out) const;
   Table_Id parse_table(std::istream &in, std::ostream &out) const;

  public:
   Readable_Parser
   (
    const Readable &readable,
    Blob_Reader *blob_reader
   ):
    readable(readable),
    blob_reader(blob_reader)
   {
   }
 };
}

#endif
