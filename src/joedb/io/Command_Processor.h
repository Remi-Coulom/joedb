#ifndef joedb_Command_Processor_declared
#define joedb_Command_Processor_declared

#include <string>
#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   enum class Status {ok, done, quit, not_found};

   virtual Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) = 0;

   virtual ~Command_Processor();
 };
}

#endif
