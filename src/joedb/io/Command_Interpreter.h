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
   Command_Interpreter()
   {
    add_processor(*static_cast<Command_Processor *>(this));
   }

   void add_processor(Command_Processor &processor)
   {
    processors.emplace_back(processor);
   }

   void set_parent(const Command_Interpreter *new_parent)
   {
    parent = new_parent;

    if (parent)
    {
     echo = parent->echo;
     rethrow = parent->rethrow;
     prompt = parent->prompt;
    }
   }

   virtual void write_prompt(std::ostream &out) const
   {
    out << "joedbi";
   }

   void write_whole_prompt(std::ostream &out) const
   {
    if (parent)
    {
     parent->write_prompt(out);
     out << '/';
    }

    write_prompt(out);
    out << "> ";
    out.flush();
   }

   void set_echo(bool new_echo) {echo = new_echo;}
   void set_rethrow(bool new_rethrow) {rethrow = new_rethrow;}
   void set_prompt(bool new_prompt) {prompt = new_prompt;}

   void main_loop(std::istream &in, std::ostream &out);
 };
}

#endif
