#include "joedb/io/run_interpreted_client.h"

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Journal_Client.h"
#include "joedb/io/Interpreter.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"

#include "joedb/concurrency/Interpreted_Client.h"

#include <iostream>
#include <memory>
#include <limits>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void run_interpreted_client(Client &client)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto *interpreted_client = dynamic_cast<Interpreted_Client *>(&client);
  auto *journal_client = dynamic_cast<Journal_Client *>(&client);

  while (std::cin)
  {
   const int64_t diff = client.get_checkpoint_difference();

   if (diff > 0)
    std::cout << "You can push " << diff << " bytes. ";
   else if (diff < 0)
    std::cout << "You can pull " << -diff << " bytes. ";

   if (interpreted_client)
    std::cout << "R(read), ";

   if (diff > 0)
    std::cout << "P(push), ";
   else
    std::cout << "P(pull), T(transaction), ";

   std::cout << "or Q(quit)? ";
   std::cout.flush();
   std::string input;

   if (!(std::cin >> input))
   {
    std::cout << '\n';
    break;
   }

   std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

   if (input == "T" && diff <= 0)
   {
    std::cout << "Waiting for lock... ";
    std::cout.flush();

    if (interpreted_client)
    {
     interpreted_client->transaction([](Readable &readable, Writable &writable)
     {
      std::cout << "OK\n";
      std::cout.flush();
      Interpreter(readable, writable, nullptr, nullptr, 0).main_loop
      (
       std::cin,
       std::cout
      );
     });
    }
    else if (journal_client)
    {
     journal_client->transaction([](Writable &writable)
     {
      std::cout << "OK\n";
      std::cout.flush();
      Writable_Interpreter(writable).main_loop
      (
       std::cin,
       std::cout
      );
     });
    }
   }
   else if (input == "P" && diff <= 0)
    client.pull();
   else if (input == "P" && diff > 0)
    client.push_unlock();
   else if (interpreted_client && input == "R")
   {
    Readable_Interpreter
    (
     interpreted_client->get_database(),
     nullptr
    )
    .main_loop(std::cin, std::cout);
   }
   else if (input == "Q")
    break;
   else
    std::cout << "Error: unknown command\n";
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void run_interpreted_client
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
