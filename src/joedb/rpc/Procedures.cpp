#include "joedb/rpc/Procedures.h"
#include "joedb/ui/type_io.h"
#include "joedb/journal/File_Hasher.h"

#include <sstream>

namespace joedb::rpc
{
 SHA_256::Hash Procedures::get_hash() const
 {
  std::ostringstream out;

  for (const Procedure * const procedure: procedures)
   write_string(out, procedure->get_prolog());

  return File_Hasher::get_hash(out.str());
 }
}
