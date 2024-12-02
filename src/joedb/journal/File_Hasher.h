#ifndef joedb_File_Hasher_declared
#define joedb_File_Hasher_declared

#include "joedb/journal/SHA_256.h"

#include <string>

namespace joedb
{
 class Generic_File;

 class File_Hasher
 {
 public:
  static SHA_256::Hash get_hash
  (
   Generic_File &file,
   int64_t start,
   int64_t size
  );

  static SHA_256::Hash get_hash(Generic_File &file);
  static SHA_256::Hash get_hash(const std::string &s);

  static SHA_256::Hash get_fast_hash
  (
   Generic_File &file,
   int64_t start,
   int64_t size
  );
 };

 class Readonly_Journal;

 class Journal_Hasher
 {
 public:
  static SHA_256::Hash get_hash
  (
   const Readonly_Journal &journal,
   int64_t checkpoint
  );
 };
}

#endif
