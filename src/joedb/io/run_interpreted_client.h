#ifndef joedb_run_interpreted_client_declared
#define joedb_run_interpreted_client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Shared_Local_File.h"
#include "joedb/io/Interpreter.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 inline void run_interpreted_client
 ////////////////////////////////////////////////////////////////////////////
 (
  Connection &connection,
  const char *file_name
 )
 {
  Shared_Local_File file(connection, file_name);
  Interpreted_Client client(connection, file);

  while (std::cin)
  {
   std::cout << "R(read), P(pull), W(write), or Q(quit)? ";
   std::cout.flush();
   std::string input;

   if (!(std::cin >> input))
   {
    std::cout << '\n';
    break;
   }

   if (input == "W")
   {
    Interpreted_Lock lock(client);
    Interpreter(lock.get_database()).main_loop(std::cin, std::cout);
   }
   else if (input == "P")
    client.pull();
   else if (input == "R")
    Readonly_Interpreter(client.get_database()).main_loop(std::cin, std::cout);
   else if (input == "Q")
    break;
  }
 }
}

#endif
