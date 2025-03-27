#include "joedb/ui/Command_Interpreter.h"
#include "joedb/journal/diagnostics.h"

#include <sstream>

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 void Command_Interpreter::after_command
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  int64_t line_number,
  const std::string &line,
  const error::Exception *exception
 ) const
 {
  if (exception)
  {
   std::ostringstream error;
   error << exception->what();
   error << "\nLine " << line_number << ": " << line << '\n';

   if (rethrow)
    throw error::Exception(error.str());
   else
    out << "Exception caught: " << error.str();
  }
  else if (echo)
   out << "OK: " << line << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Command_Interpreter::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &parameters,
  std::istream &in,
  std::ostream &out
 )
 {
  if (command.empty() || command[0] == '#') /////////////////////////////////
  {
  }
  else if (command == "about") //////////////////////////////////////////////
  {
   about_joedb(out);
  }
  else if (command == "echo") ///////////////////////////////////////////////
  {
   std::string parameter;
   parameters >> parameter;

   if (parameter == "on")
    set_echo(true);
   else if (parameter == "off")
    set_echo(false);
  }
  else if (command == "prompt") /////////////////////////////////////////////
  {
   std::string parameter;
   parameters >> parameter;

   if (parameter == "on")
    set_prompt(true);
   else if (parameter == "off")
    set_prompt(false);
  }
  else if (command == "help") ///////////////////////////////////////////////
  {
   out << R"RRR(
General commands
~~~~~~~~~~~~~~~~
 about
 help|?
 quit
 abort
 echo on|off
 prompt on|off

)RRR";

   return Status::ok;
  }
  else if (command == "quit") ///////////////////////////////////////////////
   return Status::quit;
  else if (command == "abort") //////////////////////////////////////////////
   return Status::abort;
  else
   return Status::not_found;

  return Status::done;
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Interpreter::Command_Interpreter()
 ////////////////////////////////////////////////////////////////////////////
 {
  add_processor(*static_cast<Command_Processor *>(this));
 }

 ////////////////////////////////////////////////////////////////////////////
 void Command_Interpreter::add_processor(Command_Processor &processor)
 ////////////////////////////////////////////////////////////////////////////
 {
  processors.emplace_back(processor);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Command_Interpreter::set_parent(const Command_Interpreter *new_parent)
 ////////////////////////////////////////////////////////////////////////////
 {
  parent = new_parent;

  if (parent)
  {
   echo = parent->echo;
   rethrow = parent->rethrow;
   prompt = parent->prompt;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Command_Interpreter::write_prompt(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "joedbi";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Command_Interpreter::write_whole_prompt(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
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

 ////////////////////////////////////////////////////////////////////////////
 void Command_Interpreter::main_loop(std::istream &in, std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  int64_t line_number = 0;
  bool abort = false;

  while(true)
  {
   if (prompt)
    write_whole_prompt(out);

   std::string line;
   if (!std::getline(in, line))
    break;

   line_number++;
   std::istringstream iss(line);
   std::string command;
   iss >> command;

   if (command == "?")
    command = "help";

   try
   {
    bool found = false;
    bool quit = false;

    for (const auto &processor: processors)
    {
     const Command_Processor::Status status =
      processor.get().process_command(command, iss, in, out);

     if (status != Command_Processor::Status::not_found)
      found = true;

     if (status == Command_Processor::Status::quit)
     {
      quit = true;
      break;
     }

     if (status == Command_Processor::Status::abort)
     {
      abort = true;
      break;
     }

     if (status == Command_Processor::Status::done)
      break;
    }

    if (!found)
    {
     throw error::Exception
     (
      "Unknown command. For a list of available commands, try \"help\"."
     );
    }

    after_command(out, line_number, line, nullptr);

    if (quit || abort)
     break;
   }
   catch (const error::Exception &e)
   {
    after_command(out, line_number, line, &e);
   }
  }

  if (abort)
   throw error::Exception("aborted");
 }
}
