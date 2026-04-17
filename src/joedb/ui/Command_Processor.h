#ifndef joedb_Command_Processor_declared
#define joedb_Command_Processor_declared

#include <string_view>
#include <iostream>

namespace joedb
{
 /// @ingroup ui
 class Command_Processor
 {
  public:
   enum class Status {ok, done, quit, abort, not_found};

   virtual Status process_command
   (
    std::string_view command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) = 0;

   virtual ~Command_Processor();
 };
}

#endif
