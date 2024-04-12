#include "joedb/io/Client_Command_Processor.h"
#include "joedb/io/Command_Processor.h"
#include "joedb/io/Command_Interpreter.h"
#include "joedb/io/Writable_Command_Processor.h"
#include "joedb/io/get_time_string.h"
#include "joedb/io/Interpreter.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/Signal.h"

#include <thread>
#include <chrono>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Client_Command_Processor::run_transaction
 ////////////////////////////////////////////////////////////////////////////
 (
  Writable_Interpreter &interpreter,
  std::istream &in,
  std::ostream &out
 )
 {
  out << "OK\n";
  interpreter.set_prompt(true);
  interpreter.set_prompt_string("joedb_client/transaction");
  interpreter.main_loop(in, out);

  if (interpreter.was_aborted())
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
 void Client_Command_Processor::print_status(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t diff = client.get_checkpoint_difference();

  if (diff > 0)
   out << "You can push " << diff << " bytes.\n";
  else if (diff < 0)
   out << "You can pull " << -diff << " bytes.\n";
  else
   out << "Client data is in sync with the connection.\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Command_Processor::sleep(int seconds, std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << get_time_string_of_now();
  out << ". Sleeping for " << seconds << " seconds...\n";
  for (int i = seconds; Signal::get_signal() != SIGINT && --i >= 0;)
   std::this_thread::sleep_for(std::chrono::seconds(1));
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Client_Command_Processor::is_readonly_data() const
 ////////////////////////////////////////////////////////////////////////////
 {
  return client.is_readonly();
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Client_Command_Processor::has_db() const
 ////////////////////////////////////////////////////////////////////////////
 {
  return dynamic_cast<const Interpreted_Client_Data *>(&client.get_data());
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Client_Command_Processor::process_command
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::string &command,
  std::istream &parameters,
  std::istream &in,
  std::ostream &out
 )
 {
  if (command == "help") ////////////////////////////////////////////////////
  {
   out << "Client\n";
   out << "~~~~~~\n";

   out << " status\n";
   out << " refresh\n";
   out << " push\n";
   out << " push_every <seconds>\n";

   if (!is_readonly_data())
   {
    out << " pull\n";
    out << " pull_every <seconds>\n";
    out << " transaction\n";
   }

   if (has_db())
   {
    out << " db\n";
   }

   out << '\n';
   return Status::ok;
  }
  else if (command == "status") /////////////////////////////////////////////
  {
   print_status(out);
  }
  else if (command == "refresh") ////////////////////////////////////////////
  {
   client.refresh_data();
   print_status(out);
  }
  else if (command == "pull" && !is_readonly_data()) ////////////////////////
  {
   pull(out);
  }
  else if (command == "pull_every" && !is_readonly_data()) //////////////////
  {
   int seconds = 1;
   parameters >> seconds;

   Signal::set_signal(Signal::no_signal);
   Signal::start();

   while (Signal::get_signal() != SIGINT)
   {
    pull(out);
    sleep(seconds, out);
   }
  }
  else if (command == "db" && has_db()) /////////////////////////////////////
  {
   const Interpreted_Client_Data *interpreted_client_data
   (
    dynamic_cast<const Interpreted_Client_Data *>(&client.get_data())
   );

   Readable_Interpreter interpreter
   (
    interpreted_client_data->get_database(),
    nullptr
   );

   interpreter.set_prompt(true);
   interpreter.set_prompt_string("joedb_client/db");
   interpreter.main_loop(in, out);
  }
  else if (command == "push") ///////////////////////////////////////////////
  {
   if (client.get_checkpoint_difference() > 0)
    client.push_unlock();
   else
    out << "Nothing to push\n";
  }
  else if (command == "push_every") /////////////////////////////////////////
  {
   int seconds = 1;
   parameters >> seconds;

   Signal::set_signal(Signal::no_signal);
   Signal::start();

   while (Signal::get_signal() != SIGINT)
   {
    client.refresh_data();
    if (client.get_checkpoint_difference())
     print_status(out);
    client.push_unlock();
    sleep(seconds, out);
   }
  }
  else if (command == "transaction" && !is_readonly_data()) /////////////////
  {
   out << "Waiting for lock... ";
   out.flush();

   client.transaction([&in, &out](Client_Data &data)
   {
    Writable_Interpreted_Client_Data *interpreted_client_data
    (
     dynamic_cast<Writable_Interpreted_Client_Data *>(&data)
    );

    if (interpreted_client_data)
    {
     Interpreter interpreter
     (
      interpreted_client_data->get_database(),
      interpreted_client_data->get_multiplexer(),
      nullptr,
      nullptr,
      0
     );
     run_transaction(interpreter, in, out);
    }
    else
    {
     Writable_Interpreter interpreter(data.get_writable_journal());
     run_transaction(interpreter, in, out);
    }
   });
  }
  else //////////////////////////////////////////////////////////////////////
   return Status::not_found;

  return Status::done;
 }

 ////////////////////////////////////////////////////////////////////////////
 Client_Command_Processor::Client_Command_Processor(Client &client):
 ////////////////////////////////////////////////////////////////////////////
  client(client)
 {
 }
}
