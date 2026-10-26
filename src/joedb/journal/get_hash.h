#ifndef joedb_get_hash_declared
#define joedb_get_hash_declared

#include "joedb/journal/SHA_256.h"

#include <string_view>

namespace joedb
{
 class Abstract_File;

 /// @ingroup journal
 SHA_256::Hash get_full_hash
 (
  const Abstract_File &file,
  int64_t from,
  int64_t until
 );

 /// @ingroup journal
 SHA_256::Hash get_fast_hash
 (
  const Abstract_File &file,
  int64_t from,
  int64_t until
 );

 /// @ingroup journal
 SHA_256::Hash get_full_hash(const Abstract_File &file);

 /// @ingroup journal
 SHA_256::Hash get_full_hash(std::string_view s);
}

#endif
