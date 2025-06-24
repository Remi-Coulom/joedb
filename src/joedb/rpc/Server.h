#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/rpc/Procedure.h"

#include <vector>
#include <functional>

namespace joedb::rpc
{
 class Server
 {
  private:
   const std::vector<std::reference_wrapper<Procedure>> &procedures;

  public:
   Server
   (
    const std::vector<std::reference_wrapper<Procedure>> &procedures
   );
 };
}

#endif
