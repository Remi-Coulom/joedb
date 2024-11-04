#ifndef joedb_write_value_declared
#define joedb_write_value_declared

#include "joedb/index_types.h"

#include <iosfwd>

namespace joedb
{
 class Readable;
 class Blob_Reader;

 ////////////////////////////////////////////////////////////////////////////
 void write_value
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  const Readable &readable,
  Blob_Reader *blob_reader,
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id
 );
}

#endif