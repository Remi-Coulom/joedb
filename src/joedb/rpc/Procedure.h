#ifndef joedb_rpc_Procedure_declared
#define joedb_rpc_Procedure_declared

#include "joedb/journal/Buffered_File.h"

#include <string_view>

namespace joedb::rpc
{
 class Procedure
 {
  private:
   const std::string_view schema;

  public:
   Procedure(std::string_view schema): schema(schema) {}
   virtual void execute(joedb::Buffered_File &file) = 0;
   virtual ~Procedure() = default;
 };
}

#endif
