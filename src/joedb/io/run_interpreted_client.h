#ifndef joedb_run_interpreted_client_declared
#define joedb_run_interpreted_client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Shared_Local_File.h"
#include "joedb/io/Interpreter.h"
#include "joedb/journal/Memory_File.h"

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
  std::unique_ptr<Shared_Local_File> shared_local_file;
  Memory_File memory_file;

  Generic_File *file;

  if (file_name && *file_name)
  {
   shared_local_file.reset(new Shared_Local_File(connection, file_name));
   file = &shared_local_file->get_file();
  }
  else
   file = &memory_file;

  Interpreted_Client client(connection, *file);

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
    client.write_transaction([](Readable &readable, Writable &writable)
    {
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
}

#endif
