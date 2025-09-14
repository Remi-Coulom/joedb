#ifndef joedb_rpc_Procedure_declared
#define joedb_rpc_Procedure_declared

#include "joedb/journal/Abstract_File.h"

namespace joedb::rpc
{
 /// Procedure to be executed by joedb::rpc::Server
 ///
 /// @ingroup rpc
 class Procedure
 {
  public:
   virtual void execute(joedb::Abstract_File &file) const = 0;
   virtual ~Procedure() = default;
 };
}

#endif
