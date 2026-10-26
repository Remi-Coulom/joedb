#include "joedb/rpc/get_hash.h"
#include "joedb/ui/type_io.h"
#include "joedb/journal/get_hash.h"

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

  return get_full_hash(out.str());
 }
}
