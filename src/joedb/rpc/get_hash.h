#ifndef joedb_rpc_get_hash_declared
#define joedb_rpc_get_hash_declared

#include "joedb/rpc/Signature.h"
#include "joedb/journal/SHA_256.h"

#include <vector>

namespace joedb::rpc
{
 /// Compute hash code for a collection of procedure signatures
 ///
 /// @ingroup rpc
 SHA_256::Hash get_hash(const std::vector<Signature> &signatures);
}

#endif
