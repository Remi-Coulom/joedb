#include "joedb/io/Command_Interpreter.h"
#include "joedb/journal/diagnostics.h"

#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Command_Interpreter::after_command
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  int64_t line_number,
  const std::string &line,
  const Exception *exception
 ) const
 {
  if (exception)
  {
   std::ostringstream error;
   error << exception->what();
   error << "\nLine " << line_number << ": " << line << '\n';

   if (rethrow)
    throw Exception(error.str());
   else
    out << "Error: " << error.str();
  }
  else if (echo)
   out << "OK: " << line << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Command_Interpreter::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &iss,
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
   iss >> parameter;
   
   if (parameter == "on")
    set_echo(true);
   else if (parameter == "off")
    set_echo(false);
  }
  else if (command == "help") ///////////////////////////////////////////////
  {
   out << '\n';
   out << "General commands\n";
   out << "~~~~~~~~~~~~~~~~\n";
   out << " about\n";
   out << " help\n";
   out << " quit\n";
   out << " echo on|off\n";
   out << '\n';

   return Status::ok;
  }
  else if (command == "quit") ///////////////////////////////////////////////
   return Status::quit;
  else
   return Status::not_found;

  return Status::done;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Command_Interpreter::main_loop(std::istream &in, std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  int64_t line_number = 0;

  std::string line;

  while(std::getline(in, line))
  {
   line_number++;
   std::istringstream iss(line);
   std::string command;
   iss >> command;

   try
   {
    bool found = false;
    bool quit = false;

    for (const auto &processor: processors)
    {
     const auto status = processor.get().process_command(command, iss, out);

     if (status != Command_Processor::Status::not_found)
      found = true;

     if (status == Command_Processor::Status::quit)
     {
      quit = true;
      break;
     }

     if (status == Command_Processor::Status::done)
      break;
    }

    if (!found)
    {
     throw Exception
     (
      "Unknown command. For a list of available commands, try \"help\"."
     );
    }

    after_command(out, line_number, line, nullptr);

    if (quit)
     break;
   }
   catch (const Exception &e)
   {
    after_command(out, line_number, line, &e);
   }
  }
 }
}
