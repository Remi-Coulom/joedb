#include "joedb/rpc/get_hash.h"
#include "joedb/ui/type_io.h"
#include "joedb/journal/File_Hasher.h"

#include <sstream>

namespace joedb::rpc
{
 SHA_256::Hash get_hash(const std::vector<Signature> &signatures)
 {
  std::ostringstream out;

  for (const Signature &signature: signatures)
  {
   write_string(out, signature.name);
   write_string(out, signature.prolog);
  }

  return File_Hasher::get_hash(out.str());
 }
}
