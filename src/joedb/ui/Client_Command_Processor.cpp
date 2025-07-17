#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/ui/Command_Processor.h"
#include "joedb/ui/Command_Interpreter.h"
#include "joedb/ui/get_time_string.h"
#include "joedb/ui/Interpreter.h"
#include "joedb/ui/type_io.h"
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
 ////////////////////////////////////////////////////////////////////////////
 void Client_Command_Processor::write_prompt(std::ostream &out) const
 ////////////////////////////////////////////////////////////////////////////
 {
  out << get_name() << '(';

  const int64_t journal_checkpoint = client.get_journal_checkpoint();
  const int64_t connection_checkpoint = client.get_connection_checkpoint();
  const int64_t diff = connection_checkpoint - journal_checkpoint;

  out << journal_checkpoint;
  if (diff > 0)
   out << '+' << diff << ")(pull to sync)";
  else if (diff < 0)
  {
   out << diff << ')';

   if (client.is_pullonly())
    out << "(cannot push)";
   else
    out << "(push to sync)";
  }
  else
  {
   out << ')';
   if (client.get_journal().get_position() > journal_checkpoint)
    out << '*';
  }
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
  {
   out << "pulled " << byte_count << " bytes, checkpoint = ";
   out << client.get_journal_checkpoint() << '\n';
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_Command_Processor::sleep(int seconds, std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (seconds > 0)
  {
   out << get_time_string_of_now();
   out << ". Sleeping for " << seconds << " seconds...\n";
   out.flush();
   for (int i = seconds; Signal::get_signal() != SIGINT && --i >= 0;)
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
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

   out << R"RRR(Client
~~~~~~
 db
 pull [<wait_seconds>]
 pull_every [<wait_seconds>] [<sleep_seconds>]
)RRR";
   if (!client.is_pullonly())
    out << " push\n push_every [<sleep_seconds>]\n";
   out << '\n';

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
    Readable_Interpreter interpreter(*database);
    interpreter.set_prompt_string("db");
    run_interpreter(interpreter, in, out);
   }
   else
   {
    Command_Interpreter interpreter;
    interpreter.set_prompt_string("db(blobs)");
    run_interpreter(interpreter, in, out);
   }
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
   int sleep_seconds = 0;
   parameters >> wait_seconds >> sleep_seconds;

   Signal::set_signal(Signal::no_signal);
   Signal::start();

   while (Signal::get_signal() != SIGINT)
   {
    pull(out, std::chrono::milliseconds(std::lround(wait_seconds * 1000)));
    sleep(sleep_seconds, out);
   }
  }
  else if (command == "push" && !client.is_pullonly()) //////////////////////
  {
   client.push_if_ahead();
  }
  else if (command == "push_every" && !client.is_pullonly()) ////////////////
  {
   int sleep_seconds = 1;
   parameters >> sleep_seconds;

   Signal::set_signal(Signal::no_signal);
   Signal::start();

   while (Signal::get_signal() != SIGINT)
   {
    client.push_if_ahead();
    sleep(sleep_seconds, out);
   }
  }
  else //////////////////////////////////////////////////////////////////////
   return Command_Interpreter::process_command(command, parameters, in, out);

  return Status::done;
 }

 ////////////////////////////////////////////////////////////////////////////
 Command_Processor::Status Writable_Client_Command_Processor::process_command
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
   Client_Command_Processor::process_command(command, parameters, in, out);

   out << R"RRR(Writable Client
~~~~~~~~~~~~~~~
 transaction
 set_valid_data <true|false>
 set_timestamp <true|false>
 set_hard_checkpoint <true|false>

)RRR";

   return Status::ok;
  }
  else if (command == "transaction") ////////////////////////////////////////
  {
   auto * const wdc = dynamic_cast<Writable_Database_Client *>(&client);
   auto * const wjc = dynamic_cast<Writable_Journal_Client *>(&client);

   if (wdc)
   {
    wdc->transaction([&](const Readable &readable, Writable &writable)
    {
     Interpreter interpreter(readable, writable, Record_Id::null);
     interpreter.set_prompt_string("transaction");
     run_interpreter(interpreter, in, out);
    });
   }
   else if (wjc)
   {
    wjc->transaction([&](Writable_Journal &journal)
    {
     Writable_Interpreter interpreter(journal);
     interpreter.set_prompt_string("transaction(journal)");
     run_interpreter(interpreter, in, out);
    });
   }
   else
    out << "Client is not writable, cannot run transaction\n";
  }
  else if (command == "set_valid_data") /////////////////////////////////////
  {
   get_writable_client().set_valid_data(read_boolean(parameters));
  }
  else if (command == "set_timestamp") //////////////////////////////////////
  {
   get_writable_client().set_timestamp(read_boolean(parameters));
  }
  else if (command == "set_hard_checkpoint") ////////////////////////////////
  {
   get_writable_client().set_hard_checkpoint(read_boolean(parameters));
  }
  else //////////////////////////////////////////////////////////////////////
   return Client_Command_Processor::process_command(command, parameters, in, out);

  return Status::done;
 }
}
