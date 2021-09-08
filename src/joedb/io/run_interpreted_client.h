#ifndef joedb_run_interpreted_client_declared
#define joedb_run_interpreted_client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/io/Interpreter.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"

#include <iostream>
#include <memory>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 inline void run_interpreted_client(Interpreted_Client &client)
 ////////////////////////////////////////////////////////////////////////////
 {
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

   std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

   if (input == "W")
   {
    std::cout << "connecting...";
    std::cout.flush();

    client.transaction([](Readable &readable, Writable &writable)
    {
     std::cout << " OK\n";
     std::cout.flush();
     Interpreter(readable, writable).main_loop(std::cin, std::cout);
    });
   }
   else if (input == "P")
    client.pull();
   else if (input == "R")
    Readonly_Interpreter(client.get_database()).main_loop(std::cin, std::cout);
   else if (input == "Q")
    break;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 inline void run_interpreted_client
 ////////////////////////////////////////////////////////////////////////////
 (
  Connection &connection,
  const char *file_name
 )
 {
  std::unique_ptr<Generic_File> local_file;

  if (file_name && *file_name)
   local_file.reset(new File(file_name, Open_Mode::shared_write));
  else
   local_file.reset(new Memory_File());

  Interpreted_Client client(connection, *local_file);
  run_interpreted_client(client);
 }
}

#endif
