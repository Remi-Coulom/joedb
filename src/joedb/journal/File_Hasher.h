#ifndef joedb_File_Hasher_declared
#define joedb_File_Hasher_declared

#include "joedb/journal/SHA_256.h"

namespace joedb
{
 class Generic_File;

 struct File_Hasher
 {
  static SHA_256::Hash get_hash
  (
   Generic_File &file,
   int64_t start,
   int64_t size
  );

  static SHA_256::Hash get_hash
  (
   Generic_File &file
  );

  static SHA_256::Hash get_fast_hash
  (
   Generic_File &file,
   int64_t start,
   int64_t size
  );
 };

 class Readonly_Journal;

 struct Journal_Hasher
 {
  static SHA_256::Hash get_hash
  (
   const Readonly_Journal &journal,
   int64_t checkpoint
  );
 };
}

#endif
