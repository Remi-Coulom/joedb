#include "Compiler_Options_io.h"
#include "Compiler_Options.h"

#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
bool joedb::parse_compiler_options
/////////////////////////////////////////////////////////////////////////////
(
 std::istream &in,
 Compiler_Options &compiler_options
)
{
 std::string line;

 while(std::getline(in, line))
 {
  std::istringstream iss(line);
  std::string command;
  iss >> command;

  if (command.size() == 0 || command[0] == '#')
   continue;
  if (command == "namespace")
  {
   std::string namespace_name;
   iss >> namespace_name;
   compiler_options.set_namespace_name(namespace_name);
  }
  else if (command == "create_index")
  {
  }
  else
   return false;
 }

 return true;
}
