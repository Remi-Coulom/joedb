#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/ui/Command_Processor.h"
#include "joedb/ui/Command_Interpreter.h"
#include "joedb/ui/get_time_string.h"
#include "joedb/concurrency/Client.h"
#if 0
#include "joedb/ui/Interpreter.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Readonly_Journal_Client.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Readonly_Database_Client.h"
#endif
#include "joedb/Signal.h"

#include <thread>
#include <chrono>
#include <cmath>

namespace joedb
{
 using CCP = Client_Command_Processor;

 ////////////////////////////////////////////////////////////////////////////
 void CCP::write_prompt(std::ostream &out) const
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
 void CCP::pull
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
 void CCP::sleep(int seconds, std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << get_time_string_of_now();
  out << ". Sleeping for " << seconds << " seconds...\n";
  out.flush();
  for (int i = seconds; Signal::get_signal() != SIGINT && --i >= 0;)
   std::this_thread::sleep_for(std::chrono::seconds(1));
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status CCP::process_command
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
   out << " pull [<wait_seconds>]\n";

   if (!client.is_readonly())
    out << " pull_every [<wait_seconds>] [<sleep_seconds>]\n";

   if (!client.is_pullonly())
   {
    out << " push\n";

    if (client.is_readonly())
     out << " push_every [<seconds>]\n";
    else
     out << " transaction\n";
   }

   out << " db\n";

   out << '\n';
   return Status::ok;
  }
  else if (command == "db") /////////////////////////////////////////////////
  {
#if 0
   Interpreted_Client_Data *interpreted_client_data
   (
    dynamic_cast<Interpreted_Client_Data *>(&client.get_data())
   );

   if (has_file)
   {
    Readable_Interpreter interpreter
    (
     interpreted_client_data->get_database(),
     &interpreted_client_data->get_readonly_journal()
    );

    interpreter.set_parent(this);
    interpreter.main_loop(in, out);
   }
   else
   {
    Command_Interpreter interpreter;
    Blob_Reader_Command_Processor processor
    (
     interpreted_client_data->get_readonly_journal()
    );
    interpreter.add_processor(processor);
    interpreter.set_parent(this);
    interpreter.main_loop(in, out);
   }
#endif
  }
  else if (command == "push" && !client.is_pullonly()) //////////////////////
  {
   client.push_unlock();
  }
  else if (command == "push_every" && client.is_readonly() && !client.is_pullonly())
  {
   int seconds = 1;
   parameters >> seconds;

   Signal::set_signal(Signal::no_signal);
   Signal::start();

   client.push_and_keep_locked();

   while (Signal::get_signal() != SIGINT)
   {
    sleep(seconds, out);
    client.pull();
    if (client.get_checkpoint_difference() > 0)
     client.push_and_keep_locked();
   }

   client.push_unlock();
  }
  else if (command == "pull") ///////////////////////////////////////////////
  {
   float wait_seconds = 0;
   parameters >> wait_seconds;
   pull(out, std::chrono::milliseconds(std::lround(wait_seconds * 1000)));
  }
  else if (command == "pull_every" && !client.is_readonly()) ////////////////
  {
   float wait_seconds = 1;
   int sleep_seconds = 1;
   parameters >> wait_seconds >> sleep_seconds;

   Signal::set_signal(Signal::no_signal);
   Signal::start();

   while (Signal::get_signal() != SIGINT)
   {
    pull(out, std::chrono::milliseconds(std::lround(wait_seconds * 1000)));
    sleep(sleep_seconds, out);
   }
  }
  else if (command == "transaction" && !client.is_readonly() && !client.is_pullonly())
  {
#if 0
   out << "Waiting for lock... ";
   out.flush();

   client.transaction([&]()
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
#endif
  }
  else //////////////////////////////////////////////////////////////////////
   return Command_Interpreter::process_command(command, parameters, in, out);

  return Status::done;
 }
}
