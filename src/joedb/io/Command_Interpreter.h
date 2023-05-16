#ifndef joedb_Command_Interpreter_declared
#define joedb_Command_Interpreter_declared

#include "joedb/io/Command_Processor.h"
#include "joedb/Exception.h"

#include <vector>
#include <memory>
#include <functional>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Command_Interpreter: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::reference_wrapper<Command_Processor>> processors;

   bool echo = true;
   bool rethrow = false;

   void after_command
   (
    std::ostream &out,
    int64_t line_number,
    const std::string &line,
    const Exception *exception
   ) const;

   Status process_command
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   ) final;

  public:
   Command_Interpreter()
   {
    add_processor(*this);
   }

   void set_echo(bool new_echo) {echo = new_echo;}
   void set_rethrow(bool new_rethrow) {rethrow = new_rethrow;}

   void add_processor(Command_Processor &processor)
   {
    processors.emplace_back(processor);
   }

   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
