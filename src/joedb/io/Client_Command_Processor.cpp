#include "joedb/io/Client_Command_Processor.h"
#include "joedb/io/Command_Processor.h"
#include "joedb/io/Command_Interpreter.h"
#include "joedb/io/Writable_Command_Processor.h"
#include "joedb/io/print_date.h"
#include "joedb/io/Interpreter.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Journal_Client_Data.h"
#include "joedb/Signal.h"

#include <thread>
#include <chrono>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Client_Command_Processor::run_transaction
 ////////////////////////////////////////////////////////////////////////////
 (
  Command_Interpreter &interpreter,
  Writable_Command_Processor &command_processor
 )
 {
  std::cout << "OK\n";
  interpreter.set_prompt(true);
  interpreter.set_prompt_string("joedb_client/transaction");
  interpreter.main_loop
  (
   std::cin,
   std::cout
  );

  if (command_processor.was_aborted())
   throw Exception("aborted");
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Command_Processor::pull(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t client_checkpoint = client.get_checkpoint();
  const int64_t server_checkpoint = client.pull();
  if (server_checkpoint > client_checkpoint)
   out << "pulled " << server_checkpoint - client_checkpoint << " bytes\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Client_Command_Processor::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &iss,
  std::ostream &out
 )
 {
  if (command == "help") ////////////////////////////////////////////////////
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
  else if (command == "pull") ///////////////////////////////////////////////
  {
   pull(out);
  }
  else if (command == "pull_every") /////////////////////////////////////////
  {
   int seconds = 1;
   iss >> seconds;

   Signal::signal = Signal::no_signal;
   Signal::start();

   while (Signal::signal != SIGINT)
   {
    pull(out);
    print_date(out);
    out << "Sleeping for " << seconds << " seconds...\n";

    for (int i = seconds; Signal::signal != SIGINT && --i >= 0;)
     std::this_thread::sleep_for(std::chrono::seconds(1));
   }

   Signal::stop();
  }
  else if (command == "push") ///////////////////////////////////////////////
  {
   client.push_unlock();
  }
  else if (command == "db") /////////////////////////////////////////////////
  {
   if (interpreted_client_data)
   {
    Readable_Interpreter interpreter
    (
     interpreted_client_data->get_database(),
     nullptr
    );

    interpreter.set_prompt(true);
    interpreter.set_prompt_string("joedb_client/db");
    interpreter.main_loop(std::cin, std::cout);
   }
   else
    out << "Cannot read: no table data\n";
  }
  else if (command == "transaction") ////////////////////////////////////////
  {
   std::cout << "Waiting for lock... ";
   std::cout.flush();

   if (interpreted_client_data)
   {
    client.transaction([this]()
    {
     Interpreter interpreter
     (
      interpreted_client_data->get_database(),
      interpreted_client_data->get_multiplexer(),
      nullptr,
      nullptr,
      0
     );
     run_transaction(interpreter, interpreter);
    });
   }
   else if (journal_client_data)
   {
    client.transaction([this]()
    {
     Writable_Interpreter interpreter(journal_client_data->get_journal());
     run_transaction(interpreter, interpreter);
    });
   }
  }
  else //////////////////////////////////////////////////////////////////////
   return Status::not_found;

  return Status::done;
 }

 ////////////////////////////////////////////////////////////////////////////
 Client_Command_Processor::Client_Command_Processor(Client &client):
 ////////////////////////////////////////////////////////////////////////////
  client(client),
  interpreted_client_data
  (
   dynamic_cast<Interpreted_Client_Data *>(&client.get_data())
  ),
  journal_client_data
  (
   dynamic_cast<Journal_Client_Data *>(&client.get_data())
  )
 {
 }
}
