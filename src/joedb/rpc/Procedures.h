#ifndef joedb_rpc_Procedures_declared
#define joedb_rpc_Procedures_declared

#include "joedb/rpc/Procedure.h"
#include "joedb/journal/SHA_256.h"

#include <vector>

namespace joedb::rpc
{
 class Procedures
 {
  protected:
   std::vector<Procedure *> procedures;

  public:
   SHA_256::Hash get_hash() const;
   const std::vector<Procedure *> &get_procedures() {return procedures;}
   size_t size() const {return procedures.size();}
 };
}

#endif
