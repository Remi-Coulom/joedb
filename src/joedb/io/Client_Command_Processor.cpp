#include "joedb/io/Client_Command_Processor.h"
#include "joedb/io/Command_Processor.h"
#include "joedb/io/Command_Interpreter.h"
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
 void Client_Command_Processor::write_prompt(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "joedb_client(";

  const int64_t client_checkpoint = client.get_checkpoint();
  const int64_t server_checkpoint = client.get_server_checkpoint();

  out << client_checkpoint;
  if (client_checkpoint < server_checkpoint)
   out << '+' << server_checkpoint - client_checkpoint << ")(you can pull";
  else if (server_checkpoint < client_checkpoint)
   out << '-' << client_checkpoint - server_checkpoint << ")(you can push";

  out << ')';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Command_Processor::pull
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  std::chrono::milliseconds wait
 )
 {
  const int64_t byte_count = client.pull(wait);
  if (byte_count > 0)
   out << "pulled " << byte_count << " bytes\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Command_Processor::sleep(int seconds, std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << get_time_string_of_now();
  out << ". Sleeping for " << seconds << " seconds...\n";
  out.flush();
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
   Command_Interpreter::process_command(command, parameters, in, out);

   out << "Client\n";
   out << "~~~~~~\n";
   out << " pull [<wait_milliseconds>]\n";

   if (!is_readonly_data())
    out << " pull_every [<seconds>]\n";

   if (push_client)
   {
    out << " push\n";

    if (is_readonly_data())
     out << " push_every [<seconds>]\n";
    else
     out << " transaction\n";
   }

   if (has_file)
    out << " db\n";

   out << '\n';
   return Status::ok;
  }
  else if (has_file && command == "db") //////////////////////////////////////
  {
   Interpreted_Client_Data *interpreted_client_data
   (
    dynamic_cast<Interpreted_Client_Data *>(&client.get_data())
   );

   Readable_Interpreter interpreter
   (
    interpreted_client_data->get_database(),
    &interpreted_client_data->get_readonly_journal()
   );

   interpreter.set_parent(this);
   interpreter.main_loop(in, out);
  }
  else if (command == "push" && push_client) ////////////////////////////////
  {
   push_client->push_unlock();
  }
  else if (command == "push_every" && is_readonly_data() && push_client) ////
  {
   int seconds = 1;
   parameters >> seconds;

   Signal::set_signal(Signal::no_signal);
   Signal::start();

   push_client->push_and_keep_locked();

   while (Signal::get_signal() != SIGINT)
   {
    sleep(seconds, out);
    client.pull();
    if (client.get_checkpoint_difference() > 0)
     push_client->push_and_keep_locked();
   }

   push_client->push_unlock();
  }
  else if (command == "pull") ///////////////////////////////////////////////
  {
   int64_t wait_milliseconds = 0;
   parameters >> wait_milliseconds;
   pull(out, std::chrono::milliseconds(wait_milliseconds));
  }
  else if (command == "pull_every" && !is_readonly_data()) //////////////////
  {
   int seconds = 1;
   parameters >> seconds;

   Signal::set_signal(Signal::no_signal);
   Signal::start();

   while (Signal::get_signal() != SIGINT)
    pull(out, std::chrono::seconds(seconds));
  }
  else if (command == "transaction" && !is_readonly_data() && push_client) //
  {
   out << "Waiting for lock... ";
   out.flush();

   push_client->transaction([&](Client_Data &data)
   {
    out << "OK\n";

    Writable_Interpreted_Client_Data *interpreted_client_data
    (
     dynamic_cast<Writable_Interpreted_Client_Data *>(&data)
    );

    if (interpreted_client_data)
    {
     if (has_file)
     {
      Interpreter interpreter
      (
       interpreted_client_data->get_database(),
       interpreted_client_data->get_multiplexer(),
       &interpreted_client_data->get_readonly_journal(),
       interpreted_client_data->get_multiplexer(),
       0
      );
      interpreter.set_parent(this);
      interpreter.main_loop(in, out);
     }
     else
     {
      auto &journal = interpreted_client_data->get_writable_journal();
      journal.seek_to_checkpoint();
      Writable_Interpreter interpreter(journal, journal);
      interpreter.set_parent(this);
      interpreter.main_loop(in, out);
     }
    }
   });
  }
  else //////////////////////////////////////////////////////////////////////
   return Command_Interpreter::process_command(command, parameters, in, out);

  return Status::done;
 }

 ////////////////////////////////////////////////////////////////////////////
 Client_Command_Processor::Client_Command_Processor
 ////////////////////////////////////////////////////////////////////////////
 (
  Pullonly_Client &client,
  bool has_file
 ):
  client(client),
  has_file(has_file),
  push_client(client.get_push_client())
 {
 }
}
