#ifndef joedb_Command_Interpreter_declared
#define joedb_Command_Interpreter_declared

#include "joedb/io/Command_Processor.h"
#include "joedb/Exception.h"

#include <initializer_list>
#include <vector>
#include <memory>
#include <functional>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Command_Interpreter: private Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::vector<std::reference_wrapper<Command_Processor>> processors;

   bool echo = true;
   bool rethrow = false;
   bool prompt = false;
   std::string prompt_string = "joedb";

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
   Command_Interpreter
   (
    std::initializer_list<std::reference_wrapper<Command_Processor>> list
   )
   {
    processors.emplace_back(*static_cast<Command_Processor *>(this));
    processors.insert(processors.end(), list.begin(), list.end());
   }

   void set_echo(bool new_echo) {echo = new_echo;}
   void set_rethrow(bool new_rethrow) {rethrow = new_rethrow;}
   void set_prompt(bool new_prompt) {prompt = new_prompt;}
   void set_prompt_string(std::string new_prompt_string)
   {
    prompt_string = std::move(new_prompt_string);
   }

   const std::string &get_prompt_string() const {return prompt_string;}

   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
