#ifndef joedb_rpc_Procedure_declared
#define joedb_rpc_Procedure_declared

#include "joedb/journal/Buffered_File.h"

#include <string>

namespace joedb::rpc
{
 /// Procedure to be executed by joedb::rpc::Server
 ///
 /// @ingroup RPC
 class Procedure
 {
  private:
   const std::string prolog;

  public:
   Procedure(std::string prolog): prolog(std::move(prolog)) {}
   const std::string &get_prolog() const {return prolog;}
   virtual void execute(joedb::Buffered_File &file) = 0;
   virtual ~Procedure() = default;
 };
}

#endif
