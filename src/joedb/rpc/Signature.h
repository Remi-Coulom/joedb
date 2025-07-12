#ifndef joedb_rpc_Signature_declared
#define joedb_rpc_Signature_declared

#include <string>

namespace joedb::rpc
{
 /// Signature of a procedure, used both on client and server side
 ///
 /// @ingroup rpc
 struct Signature
 {
  const std::string name;
  const std::string prolog;
 };
}

#endif
