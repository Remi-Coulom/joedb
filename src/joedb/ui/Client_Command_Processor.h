#ifndef joedb_Client_Command_Processor_declared
#define joedb_Client_Command_Processor_declared

#include "joedb/ui/Command_Interpreter.h"
#include "joedb/ui/Blob_Reader_Command_Processor.h"
#include "joedb/concurrency/Writable_Client.h"

#include <chrono>

namespace joedb
{
 class Client;

 /// @ingroup ui
 class Client_Command_Processor: public Command_Interpreter
 {
  private:
   Blob_Reader_Command_Processor blob_processor;

   void pull(std::ostream &out, std::chrono::milliseconds wait);
   void print_status(std::ostream &out);

  protected:
   Client &client;

   static void sleep(int seconds, std::ostream &out);
   virtual std::string get_name() const {return "readonly_client";}

  public:
   Client_Command_Processor(Client &client):
    blob_processor(client.get_journal().get_file()),
    client(client)
   {}

   void write_prompt(std::ostream &out) const override;

   void run_interpreter
   (
    Command_Interpreter &interpreter,
    std::istream &in,
    std::ostream &out
   )
   {
    interpreter.add_processor(blob_processor);
    interpreter.set_parent(this);
    interpreter.main_loop(in, out);
   }

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;
 };

 /// @ingroup ui
 class Writable_Client_Command_Processor: public Client_Command_Processor
 {
  private:
   Writable_Client &get_writable_client()
   {
    return static_cast<Writable_Client&>(Client_Command_Processor::client);
   }

   std::string get_name() const override {return "writable_client";}

  public:
   Writable_Client_Command_Processor(Writable_Client &client):
    Client_Command_Processor(client)
   {
   }

   Status process_command
   (
    const std::string &command,
    std::istream &parameters,
    std::istream &in,
    std::ostream &out
   ) override;
 };
}

#endif
