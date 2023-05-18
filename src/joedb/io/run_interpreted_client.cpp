#include "joedb/io/run_interpreted_client.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/print_date.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Journal_Client.h"
#include "joedb/Signal.h"

#include <iostream>
#include <limits>
#include <thread>
#include <chrono>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static void run_transaction(Command_Interpreter &interpreter)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::cout << "OK\n";
  interpreter.set_prompt(true);
  interpreter.set_prompt_string("joedb_client/transaction");
  interpreter.main_loop
  (
   std::cin,
   std::cout
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 class Client_Command_Processor: public Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Client &client;
   Interpreted_Client * const interpreted_client;
   Journal_Client * const journal_client;

   //////////////////////////////////////////////////////////////////////////
   Status process_command
   //////////////////////////////////////////////////////////////////////////
   (
    const std::string &command,
    std::istream &iss,
    std::ostream &out
   ) override
   {
    if (command == "help") //////////////////////////////////////////////////
    {
     out << "Client\n";
     out << "~~~~~~\n";
     out << " pull\n";
     out << " pull_every <seconds>\n";
     out << " push\n";
     out << " db\n";
     out << " transaction\n";
     out << '\n';
     return Status::ok;
    }
    else if (command == "pull") /////////////////////////////////////////////
    {
     client.pull();
    }
    else if (command == "pull_every") ///////////////////////////////////////
    {
     int seconds = 1;
     iss >> seconds;

     Signal::signal = Signal::no_signal;
     Signal::start();

     while (Signal::signal != SIGINT)
     {
      client.pull();
      print_date(out);
      out << "Sleeping for " << seconds << " seconds...\n";

      for (int i = seconds; Signal::signal != SIGINT && --i >= 0;)
       std::this_thread::sleep_for(std::chrono::seconds(1));
     }

     Signal::stop();
    }
    else if (command == "push") /////////////////////////////////////////////
    {
     client.push_unlock();
    }
    else if (command == "db") ///////////////////////////////////////////////
    {
     if (interpreted_client)
     {
      Readable_Interpreter interpreter
      (
       interpreted_client->get_database(),
       nullptr
      );

      interpreter.set_prompt(true);
      interpreter.set_prompt_string("joedb_client/db");
      interpreter.main_loop(std::cin, std::cout);
     }
     else
      out << "Cannot read: no table data\n";
    }
    else if (command == "transaction") //////////////////////////////////////
    {
     std::cout << "Waiting for lock... ";
     std::cout.flush();

     if (interpreted_client)
     {
      interpreted_client->transaction
      (
       [](Readable &readable, Writable &writable
      )
      {
       Interpreter interpreter(readable, writable, nullptr, nullptr, 0);
       run_transaction(interpreter);
      });
     }
     else if (journal_client)
     {
      journal_client->transaction([](Writable &writable)
      {
       Writable_Interpreter interpreter(writable);
       run_transaction(interpreter);
      });
     }
    }
    else ////////////////////////////////////////////////////////////////////
     return Status::not_found;

    return Status::done;
   }

  public:
   Client_Command_Processor(Client &client):
    client(client),
    interpreted_client(dynamic_cast<Interpreted_Client *>(&client)),
    journal_client(dynamic_cast<Journal_Client *>(&client))
   {
   }

   bool has_db() const {return interpreted_client != nullptr;}
 };

 ////////////////////////////////////////////////////////////////////////////
 void run_interpreted_client(Client &client)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t diff = client.get_checkpoint_difference();

  if (diff > 0)
   std::cout << "You can push " << diff << " bytes.\n";
  else if (diff < 0)
   std::cout << "You can pull " << -diff << " bytes.\n";

  Client_Command_Processor processor(client);
  Command_Interpreter interpreter{processor};
  interpreter.set_prompt(true);

  if (processor.has_db())
   interpreter.set_prompt_string("joedb_client");
  else
   interpreter.set_prompt_string("joedb_client(nodb)");

  interpreter.main_loop(std::cin, std::cout);
 }
}
