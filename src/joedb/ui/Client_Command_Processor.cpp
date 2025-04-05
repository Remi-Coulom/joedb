#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/ui/Command_Processor.h"
#include "joedb/ui/Command_Interpreter.h"
#include "joedb/ui/get_time_string.h"
#include "joedb/ui/Interpreter.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/Writable_Database_Client.h"
#include "joedb/concurrency/Readonly_Database_Client.h"
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
   out << '+' << server_checkpoint - client_checkpoint << ")(pull to sync";
  else if (server_checkpoint < client_checkpoint)
   out << '-' << client_checkpoint - server_checkpoint << ")(push to sync";

  out << ')';

  if (client.is_readonly())
   out << "(readonly)";
  if (client.is_pullonly())
   out << "(pullonly)";
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

   out << R"RRR(Client
~~~~~~
 pull [<wait_seconds>]
 pull_every [<wait_seconds>] [<sleep_seconds>]
 push
 push_every [<seconds>]
 transaction
 db

)RRR";

   return Status::ok;
  }
  else if (command == "db") /////////////////////////////////////////////////
  {
   const Database *database = nullptr;

   {
    auto * const rdc = dynamic_cast<Readonly_Database_Client *>(&client);
    auto * const wdc = dynamic_cast<Writable_Database_Client *>(&client);

    if (rdc)
     database = &rdc->get_database();
    else if (wdc)
     database = &wdc->get_database();
   }

   if (database)
   {
    Readable_Interpreter interpreter(*database, &client.get_journal());
    interpreter.set_parent(this);
    interpreter.main_loop(in, out);
   }
   else
   {
    Command_Interpreter interpreter;
    Blob_Reader_Command_Processor processor(client.get_journal());
    interpreter.add_processor(processor);
    interpreter.set_parent(this);
    interpreter.main_loop(in, out);
   }
  }
  else if (command == "push") ///////////////////////////////////////////////
  {
   client.push_unlock();
  }
  else if (command == "push_every") /////////////////////////////////////////
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
  else if (command == "pull_every") /////////////////////////////////////////
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
  else if (command == "transaction") ////////////////////////////////////////
  {
   auto * const wdc = dynamic_cast<Writable_Database_Client *>(&client);
   auto * const wjc = dynamic_cast<Writable_Journal_Client *>(&client);

   if (!wdc && !wjc)
   {
    out << "Transactions are not available for this type of client\n";
   }
   else
   {
    out << "Waiting for lock... ";
    out.flush();

    if (wdc)
    {
     wdc->transaction([&](const Readable &readable, Writable &writable)
     {
      Interpreter interpreter
      (
       readable,
       writable,
       &client.get_journal(),
       writable,
       0
      );
      interpreter.set_parent(this);
      interpreter.main_loop(in, out);
     });
    }
    else if (wjc)
    {
     wjc->transaction([&](Writable_Journal &journal)
     {
      Writable_Interpreter interpreter(journal, journal);
      interpreter.set_parent(this);
      interpreter.main_loop(in, out);
     });
    }
   }
  }
  else //////////////////////////////////////////////////////////////////////
   return Command_Interpreter::process_command(command, parameters, in, out);

  return Status::done;
 }
}
