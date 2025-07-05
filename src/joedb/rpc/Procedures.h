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
   std::vector<std::string> names;

  public:
   SHA_256::Hash get_hash() const;
   const std::vector<Procedure *> &get_procedures() const {return procedures;}
   const std::vector<std::string> &get_names() const {return names;}
   size_t size() const {return procedures.size();}
 };
}

#endif
