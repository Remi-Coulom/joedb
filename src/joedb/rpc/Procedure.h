#ifndef joedb_rpc_Procedure_declared
#define joedb_rpc_Procedure_declared

#include "joedb/journal/Memory_File.h"

#include <string_view>

namespace joedb::rpc
{
 class Procedure
 {
  private:
   const std::string_view name;
   const std::string_view schema;

  public:
   Procedure
   (
    std::string_view name,
    std::string_view schema
   ):
    name(name),
    schema(schema)
   {
   }

   virtual void execute(joedb::Memory_File &file);

   virtual ~Procedure() = default;
 };
}

#endif
