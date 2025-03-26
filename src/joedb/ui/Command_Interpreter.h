#ifndef joedb_Command_Interpreter_declared
#define joedb_Command_Interpreter_declared

#include "joedb/ui/Command_Processor.h"
#include "joedb/Exception.h"

#include <vector>
#include <functional>
#include <stdint.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Command_Interpreter: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::reference_wrapper<Command_Processor>> processors;
   const Command_Interpreter *parent = nullptr;

   bool echo = true;
   bool rethrow = false;
   bool prompt = false;

   void after_command
   (
    std::ostream &out,
    int64_t line_number,
    const std::string &line,
    const Exception *exception
   ) const;

  protected:
   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;

  public:
   Command_Interpreter();
   void add_processor(Command_Processor &processor);
   void set_parent(const Command_Interpreter *new_parent);
   virtual void write_prompt(std::ostream &out) const;
   void write_whole_prompt(std::ostream &out) const;
   void set_echo(bool new_echo) {echo = new_echo;}
   void set_rethrow(bool new_rethrow) {rethrow = new_rethrow;}
   void set_prompt(bool new_prompt) {prompt = new_prompt;}
   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
